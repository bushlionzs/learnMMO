/*$T Common/EncryptAriths/safer.h GC 1.140 10/10/07 10:06:30 */


/*$6
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */


#ifndef __SAFER_H__
#define __SAFER_H__

#include "CryptDef.h"

class SAFER :
	public BlockTransformation
{
/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
public:
	enum { BLOCKSIZE = 8, MAX_ROUNDS = 13 };

	/*
	 ===============================================================================================================
	 ===============================================================================================================
	 */
	uint32 BlockSize() const
	{
		return BLOCKSIZE;
	}

/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
protected:
	void					SetUserKey
						(
							const byte	*userkey_1,
							const byte	*userkey_2,
							unsigned	nof_rounds,
							bool		strengthened
						);

	void					Encrypt(const byte *inBlock, byte *outBlock) const;
	void					Decrypt(const byte *inBlock, byte *outBlock) const;

	SecBlock<uchar, 1 +BLOCKSIZE *21>	keySchedule;
	static const byte			exp_tab[256];
	static const byte			log_tab[256];
};

class SAFER_K64_Encryption :
	public SAFER
{
/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
public:
	enum { KEYLENGTH = 8, ROUNDS = 6 };
	void SetUserKey(const byte *userKey);

	/*
	 ===============================================================================================================
	 ===============================================================================================================
	 */

	void ProcessBlock(byte *inoutBlock) const
	{
		SAFER::Encrypt(inoutBlock, inoutBlock);
	}

	/*
	 ===============================================================================================================
	 ===============================================================================================================
	 */
	void ProcessBlock(const byte *inBlock, byte *outBlock) const
	{
		SAFER::Encrypt(inBlock, outBlock);
	}
};

class SAFER_K64_Decryption :
	public SAFER
{
/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
public:
	enum { KEYLENGTH = 8, ROUNDS = 6 };

	void SetUserKey(const byte *userKey);

	/*
	 ===============================================================================================================
	 ===============================================================================================================
	 */

	void ProcessBlock(byte *inoutBlock) const
	{
		SAFER::Decrypt(inoutBlock, inoutBlock);
	}

	/*
	 ===============================================================================================================
	 ===============================================================================================================
	 */
	void ProcessBlock(const byte *inBlock, byte *outBlock) const
	{
		SAFER::Decrypt(inBlock, outBlock);
	}
};

class SAFER_K128_Encryption :
	public SAFER
{
/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
public:
	enum { KEYLENGTH = 16, ROUNDS = 10 };

	void SetUserKey(const byte *userKey);

	/*
	 ===============================================================================================================
	 ===============================================================================================================
	 */

	void ProcessBlock(byte *inoutBlock) const
	{
		SAFER::Encrypt(inoutBlock, inoutBlock);
	}

	/*
	 ===============================================================================================================
	 ===============================================================================================================
	 */
	void ProcessBlock(const byte *inBlock, byte *outBlock) const
	{
		SAFER::Encrypt(inBlock, outBlock);
	}
};

class SAFER_K128_Decryption :
	public SAFER
{
/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
public:
	enum { KEYLENGTH = 16, ROUNDS = 10 };

	void SetUserKey(const byte *userKey);

	/*
	 ===============================================================================================================
	 ===============================================================================================================
	 */

	void ProcessBlock(byte *inoutBlock) const
	{
		SAFER::Decrypt(inoutBlock, inoutBlock);
	}

	/*
	 ===============================================================================================================
	 ===============================================================================================================
	 */
	void ProcessBlock(const byte *inBlock, byte *outBlock) const
	{
		SAFER::Decrypt(inBlock, outBlock);
	}
};

class SAFER_SK64_Encryption :
	public SAFER
{
/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
public:
	enum { KEYLENGTH = 8, ROUNDS = 8 };

	void SetUserKey(const byte *userKey);

	/*
	 ===============================================================================================================
	 ===============================================================================================================
	 */

	void ProcessBlock(byte *inoutBlock) const
	{
		SAFER::Encrypt(inoutBlock, inoutBlock);
	}

	/*
	 ===============================================================================================================
	 ===============================================================================================================
	 */
	void ProcessBlock(const byte *inBlock, byte *outBlock) const
	{
		SAFER::Encrypt(inBlock, outBlock);
	}
};

class SAFER_SK64_Decryption :
	public SAFER
{
/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
public:
	enum { KEYLENGTH = 8, ROUNDS = 8 };

	void SetUserKey(const byte *userKey);

	/*
	 ===============================================================================================================
	 ===============================================================================================================
	 */

	void ProcessBlock(byte *inoutBlock) const
	{
		SAFER::Decrypt(inoutBlock, inoutBlock);
	}

	/*
	 ===============================================================================================================
	 ===============================================================================================================
	 */
	void ProcessBlock(const byte *inBlock, byte *outBlock) const
	{
		SAFER::Decrypt(inBlock, outBlock);
	}
};

class SAFER_SK128_Encryption :
	public SAFER
{
/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
public:
	enum { KEYLENGTH = 16, ROUNDS = 10 };

	void SetUserKey(const byte *userKey);

	/*
	 ===============================================================================================================
	 ===============================================================================================================
	 */

	void ProcessBlock(byte *inoutBlock) const
	{
		SAFER::Encrypt(inoutBlock, inoutBlock);
	}

	/*
	 ===============================================================================================================
	 ===============================================================================================================
	 */
	void ProcessBlock(const byte *inBlock, byte *outBlock) const
	{
		SAFER::Encrypt(inBlock, outBlock);
	}
};

class SAFER_SK128_Decryption :
	public SAFER
{
/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
public:
	enum { KEYLENGTH = 16, ROUNDS = 10 };

	void SetUserKey(const byte *userKey);

	/*
	 ===============================================================================================================
	 ===============================================================================================================
	 */

	void ProcessBlock(byte *inoutBlock) const
	{
		SAFER::Decrypt(inoutBlock, inoutBlock);
	}

	/*
	 ===============================================================================================================
	 ===============================================================================================================
	 */
	void ProcessBlock(const byte *inBlock, byte *outBlock) const
	{
		SAFER::Decrypt(inBlock, outBlock);
	}
};
#endif