// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include "OgreHeader.h"
#include <Brush.h>
#include <MapTile.h>
#include <Misc.h>
#include <World.h>
#include <texture_set.hpp>
#include <algorithm>    // std::min
#include <iostream>     // std::cout
#include <utility>
#include "OgreDataStream.h"
#include "OgreTextureManager.h"

TextureSet::TextureSet (
    MapChunkHeader const& header,
    std::shared_ptr<DataStream>& stream,
    size_t base, 
    MapTile* tile, 
    bool use_big_alphamaps, 
    bool do_not_fix_alpha_map, 
    bool do_not_convert_alphamaps)
  : nTextures(header.nLayers)
  , _do_not_convert_alphamaps(do_not_convert_alphamaps)
{
    for (int i = 0; i < 64; ++i)
    {
        const size_t array_index(i / 4);
        // it's a uint2 array so we need to read the bits as they come on the disk,
        // this means reading the highest bits from the uint8 first
        const size_t bit_index((3-((i) % 4)) * 2);

        _lod_texture_map.push_back(((header.low_quality_texture_map[array_index]) >> (bit_index)) & 3);
    }

    if (nTextures)
    {
        stream->seek(base + header.ofsLayer + 8);

        for (size_t i = 0; i<nTextures; ++i)
        {
            stream->read (&_layers_info[i], sizeof(ENTRY_MCLY));

            textures.emplace_back (tile->mTextureFilenames[_layers_info[i].textureID]);
        }

        size_t alpha_base = base + header.ofsAlpha + 8;

        for (unsigned int layer = 0; layer < nTextures; ++layer)
        {
            if (_layers_info[layer].flags & 0x100)
            {
                stream->seek (alpha_base + _layers_info[layer].ofsAlpha);
                if (!alphamaps[layer - 1].has_value())
                {
                    alphamaps[layer - 1].emplace();
                }
                alphamaps[layer - 1]->load(stream, _layers_info[layer].flags, use_big_alphamaps, do_not_fix_alpha_map);
            }
        }

        // always use big alpha for editing / rendering
        if (!use_big_alphamaps && !_do_not_convert_alphamaps)
        {
            convertToBigAlpha();
        }

        _need_amap_update = true;
    }
}


std::shared_ptr<OgreTexture> TextureSet::createAlphaMapTexture(const std::string& name)
{
    TextureProperty texProperty;
    texProperty._width = 64;
    texProperty._height = 64;
    texProperty._tex_format = PF_BYTE_RGB;

    texProperty._tex_usage = Ogre::TextureUsage::UPLOADABLE;
    auto tex = TextureManager::getSingleton().createManual(name, texProperty);

    std::vector<uint8_t> amap(3 * 64 * 64);
    uint8_t const* alpha_ptr[3];

    for (int i = 0; i < nTextures - 1; ++i)
    {
        alpha_ptr[i] = alphamaps[i]->getAlpha();
    }

    for (int i = 0; i < 64 * 64; ++i)
    {
        for (int alpha_id = 0; alpha_id < 3; ++alpha_id)
        {
            amap[i * 3 + alpha_id] = (alpha_id < nTextures - 1)
                ? *(alpha_ptr[alpha_id]++)
                : 0
                ;
        }
    }

    PixelBox src(64, 64, 1, PF_BYTE_RGB, amap.data());
    Box dst(Vector3i(tex->getWidth(), tex->getHeight(), tex->getDepth()));
    tex->blitFromMemory(src, dst);
    tex->uploadData();

    return tex;
}

const std::string& TextureSet::filename(size_t id)
{
  return textures[id];
}

Ogre::Vector2 TextureSet::anim_uv_offset(int id, int animtime) const
{
    uint32_t flags = _layers_info[id].flags;

    const int spd = (flags >> 3) & 0x7;
    const int dir = flags & 0x7;
    const float texanimxtab[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
    const float texanimytab[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
    const float fdx = -texanimxtab[dir], fdy = texanimytab[dir];
    const int animspd = (const int)(200 * detail_size);
    float f = ((static_cast<int>(animtime*(spd / 7.0f))) % animspd) / static_cast<float>(animspd);
    return { f*fdx, f*fdy };
}

unsigned int TextureSet::flag(size_t id)
{
    return _layers_info[id].flags;
}

unsigned int TextureSet::effect(size_t id)
{
    return _layers_info[id].effectID;
}

bool TextureSet::is_animated(std::size_t id) const
{
    return (id < nTextures ? (_layers_info[id].flags & FLAG_ANIMATE) : false);
}

void TextureSet::change_texture_flag(std::string const& tex, std::size_t flag, bool add)
{
  for (size_t i = 0; i < nTextures; ++i)
  {
    if (textures[i] == tex)
    {
      if (add)
      {
        // override the current speed/rotation
        if (flag & 0x3F)
        {
          _layers_info[i].flags &= ~0x3F;
        }
        _layers_info[i].flags |= flag;
      }
      else
      {
        _layers_info[i].flags &= ~flag;
      }

      if (flag & FLAG_GLOW)
      {
        _layers_info[i].flags |= FLAG_GLOW;
      }
      else
      {
        _layers_info[i].flags &= ~FLAG_GLOW;
      }

      break;
    }
  }
}

std::string TextureSet::texture(size_t id)
{
  return textures[id];
}

void TextureSet::alphas_to_big_alpha(uint8_t* dest)
{
  auto alpha
  (
    [&] (int layer, int pos = 0)
    {
      return dest + layer * 4096 + pos;
    }
  );

  for (size_t k = 0; k < nTextures - 1; k++)
  {
    memcpy(alpha(k), alphamaps[k]->getAlpha(), 4096);
  }

  for (int i = 0; i < 64 * 64; ++i)
  {
    int a = 255;

    for (int k = nTextures - 2; k >= 0; --k)
    {
      uint8_t val = misc::rounded_255_int_div(*alpha(k, i) * a);
      a -= val;
      *alpha(k, i) = val;
    }
  }
}

void TextureSet::convertToBigAlpha()
{
  // nothing to do
  if (nTextures < 2)
  {
    return;
  }

  uint8_t tab[4096 * 3];

  alphas_to_big_alpha(tab);

  for (size_t k = 0; k < nTextures - 1; k++)
  {
    alphamaps[k]->setAlpha(tab + 4096 * k);
  }
}


void TextureSet::alphas_to_old_alpha(uint8_t* dest)
{
  auto alpha
  (
    [&] (int layer, int pos = 0)
    {
      return dest + layer * 4096 + pos;
    }
  );

  for (size_t k = 0; k < nTextures - 1; k++)
  {
    memcpy(alpha(k), alphamaps[k]->getAlpha(), 64 * 64);
  }

  for (int i = 0; i < 64 * 64; ++i)
  {
    // a = remaining visibility
    int a = 255;

    for (int k = nTextures - 2; k >= 0; --k)
    {
      if (a <= 0)
      {
        *alpha(k, i) = 0;
      }
      else
      {
        int current = *alpha(k, i);
        // convert big alpha value to old alpha
        *alpha(k, i) = misc::rounded_int_div(current * 255, a);
        // remove big alpha value from the remaining visibility
        a -= current;
      }
    }
  }
}

void TextureSet::convertToOldAlpha()
{
  // nothing to do
  if (nTextures < 2)
  {
    return;
  }

  uint8_t tab[3 * 4096];

  alphas_to_old_alpha(tab);

  for (size_t k = 0; k < nTextures - 1; k++)
  {
    alphamaps[k]->setAlpha(tab + k * 4096);
  }

  _need_amap_update = true;
}

void TextureSet::bind_alpha(std::size_t id)
{
  
}

namespace
{
  misc::max_capacity_stack_vector<std::size_t, 4> current_layer_values
    (std::uint8_t nTextures, std::optional<Alphamap> const* alphamaps, std::size_t pz, std::size_t px)
  {
    misc::max_capacity_stack_vector<std::size_t, 4> values (nTextures, 0xFF);
    for (std::uint8_t i = 1; i < nTextures; ++i)
    {
      values[i] = alphamaps[i - 1]->getAlpha(64 * pz + px);
      values[0] -= values[i];
    }
    return values;
  }
}

std::vector<uint8_t> TextureSet::lod_texture_map()
{
  if (_need_lod_texture_map_update)
  {
    update_lod_texture_map();
  }

  return _lod_texture_map;
}

void TextureSet::update_lod_texture_map()
{
  std::vector<std::uint8_t> lod;

  for (std::size_t z = 0; z < 8; ++z)
  {
    for (std::size_t x = 0; x < 8; ++x)
    {
      misc::max_capacity_stack_vector<std::size_t, 4> dominant_square_count (nTextures);

      for (std::size_t pz = z * 8; pz < (z + 1) * 8; ++pz)
      {
        for (std::size_t px = x * 8; px < (x + 1) * 8; ++px)
        {
          ++dominant_square_count[max_element_index (current_layer_values (nTextures, alphamaps.data(), pz, px))];
        }
      }
      lod.push_back (max_element_index (dominant_square_count));
    }
  }

  _lod_texture_map = lod;
  _need_lod_texture_map_update = false;
}

uint8_t TextureSet::sum_alpha(size_t offset) const
{
  uint8_t sum = 0;

  for (auto const& amap : alphamaps)
  {
    if (amap)
    {
      sum += amap->getAlpha(offset);
    }
  }

  return sum;
}

namespace
{
  inline std::uint8_t float_alpha_to_uint8(float a)
  {
    return static_cast<std::uint8_t>(std::max(0.f, std::min(255.f, std::round(a))));
  }
}

