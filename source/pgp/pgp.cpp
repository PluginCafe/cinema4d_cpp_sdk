// example code for a menu plugin and multiprocessing

#include "c4d.h"
#include "c4d_symbols.h"
#include "c4d_memory_mp.h"
#include "lib_aes.h"
#include "lib_big_integer.h"
#include "lib_secure_random.h"
#include "lib_zipfile.h"
#include "main.h"

class PGPTest : public CommandData
{
	static const maxon::Int keySize = 256, blockSize = 256;
#ifdef MAXON_TARGET_DEBUG
	// lower value for speed reasons
	static const maxon::Int certainty = 20;
#else
	static const maxon::Int certainty = 300;
#endif

public:
	virtual Bool Execute(BaseDocument* doc)
	{
		const maxon::Int		origBufferSize = 1 << 20;	// 1 MiB
		maxon::UChar*					transmitData = nullptr;
		maxon::UChar*					receiveData	 = nullptr;
		maxon::Int						transmitDataSize = 0, receiveDataSize = 0, j;
		AutoAlloc<BigInteger> publicKeyReceiver, privateKeyReceiver, nReceiver, encryptedSymmetricKey;
		AutoAlloc<BigInteger> publicKeySender, privateKeySender, nSender, signature;

		if (!publicKeyReceiver || !privateKeyReceiver || !nReceiver ||
				!publicKeySender || !privateKeySender || !nSender ||
				!encryptedSymmetricKey || !signature)
			return false;

		UInt32 origCRC, newCRC;
		SecureRandomProvider provider;
		maxon::Bool signatureOK1 = false, signatureOK2 = false;

		// set up our original data
		AutoGeFree<maxon::UChar> data(NewMem(maxon::UChar, origBufferSize));
		if (!data)
			return false;
		provider = SecureRandom::GetDefaultProvider();
		SecureRandom::GetRandomNumber(provider, data, origBufferSize);
		// Only use numbers, letters and spaces so that compression actually does something. With good random numbers the entropy of the buffer is 1 and there is nothing to do.
		for (j = 0; j < origBufferSize; j++)
		{
			data[j] &= 127;
			if (!((data[j] >= '0' && data[j] <= '9') || (data[j] >= 'a' && data[j] <= 'z') || (data[j] >= 'A' && data[j] <= 'Z')))
				data[j] = ' ';
		}
		origCRC = ZipFile::CalcCRC32(data, origBufferSize);

		// Generate a public (publicKeyReceiver, nReceiver) and a private (privateKeyReceiver, nReceiver) key pair for the receiver.
		if (!GenerateAsymmetricKeys(publicKeyReceiver, privateKeyReceiver, nReceiver))
			return false;

		// Generate a public (publicKeySender, nSender) and a private (privateKeySender, nSender) key pair for the sender.
		if (!GenerateAsymmetricKeys(publicKeySender, privateKeySender, nSender))
			return false;

		// Encrypt the data using the receiver's public key and create a symmetric key for encryption.
		if (!EncryptData(data, origBufferSize, publicKeyReceiver, nReceiver, encryptedSymmetricKey, transmitData, transmitDataSize))
			return false;

		// Create a signature so that the receiver can be sure that the data originally came from the sender.
		if (!CreateSignature(data, origBufferSize, privateKeySender, nSender, signature))
			return false;

		// Now, send transmitData, encryptedSymmetricKey and signature to the receiver.

		// The receiver decrypts the data using the receiver's private key.
		if (!DecryptData(transmitData, transmitDataSize, privateKeyReceiver, nReceiver, encryptedSymmetricKey, receiveData, receiveDataSize))
			return false;

		DeleteMem(transmitData);

		// Let's check the signature.
		if (!CheckSignature(receiveData, receiveDataSize, publicKeySender, nSender, signature, signatureOK1))
			return false;

		newCRC = ZipFile::CalcCRC32(receiveData, (Int32)receiveDataSize);

		// Modify the received data and check the signature again. It must not match now.
		receiveData[0]++;
		if (!CheckSignature(receiveData, receiveDataSize, publicKeySender, nSender, signature, signatureOK2))
			return false;

		DeleteMem(receiveData);

		Bool	 ok = (origCRC == newCRC && signatureOK1 && !signatureOK2);
		String error;
		if (ok)
		{
			error = "PGP test succeeded";
		}
		else
		{
			error = "PGP test failed: ";
			if (origCRC != newCRC)
				error += "CRC different ";
			if (!signatureOK1)
				error += "Signature 1 mismatch ";
			if (signatureOK2)
				error += "Signature 2 mismatch ";
		}
		GeOutString(error, GEMB_OK);

		return true;
	}

	Bool GenerateAsymmetricKeys(BigInteger* publicKey, BigInteger* privateKey, BigInteger* n)
	{
		SecureRandomProvider	provider;
		AutoAlloc<BigInteger> p, q, phi_n, i;
		if (!p || !q || !phi_n || !i)
			return false;

		provider = SecureRandom::GetDefaultProvider();

		// Generate a key pair for asymmetric encryption and decryption. These numbers are just created here randomly.
		// Normally, they are only created once, the result is stored and p and q are forgotten destroyed. No one may ever get knowledge about p or q.
		// p and q can be 2**128 - 1 at maximum. Their product is less than 2**256 so that they can be used for a 256-bit key.
		p->SetRandomPrime(128, certainty, provider);
		q->SetRandomPrime(128, certainty, provider);
		n->CopyFrom(p); n->Mul(q);															// n = p * q
		privateKey->SetRandomPrime(129, certainty, provider);	// take 129 bis here to make sure it's larger than max(p, q).
		p->Dec();
		q->Dec();
		phi_n->CopyFrom(p); phi_n->Mul(q);	// phi_n = p * q

		publicKey->CopyFrom(privateKey);
		publicKey->MultiplicativeInverse(phi_n);

		// just a test: a number multiplied with its inverse mod n must be 1.
		i->CopyFrom(publicKey);
		i->Mul(privateKey);
		i->Mod(phi_n);
		DebugAssert(i->IsOne());

		return true;
	}

	Bool EncryptData(const maxon::UChar* origData, maxon::Int origDataSize, const BigInteger* publicKey, const BigInteger* n, BigInteger* encryptedSymmetricKey, maxon::UChar*& transmitData, maxon::Int& transmitDataSize)
	{
		SecureRandomProvider provider;
		void* compressedData = nullptr;
		AutoAlloc<BigInteger> symmetricKey;
		AutoAlloc<AES>				crypt;
		maxon::Int					j, encryptedSize;
		maxon::BaseArray<maxon::UChar> keyArray;
		maxon::UChar	 key[blockSize / 8];
		maxon::UChar* transmit;
		Int compressedDataSize = 0;

		if (!symmetricKey || !crypt)
			return false;

		provider = SecureRandom::GetDefaultProvider();

		// generate a key for the symmetric encryption and decryption. This must be smaller than n. A number mod n will always be >= 0 and < n.
		do
		{
			symmetricKey->SetRandom(256);
		} while (symmetricKey->Compare(n) != maxon::COMPARERESULT_LESS);

		// encrypt this symmetric key using our private asymmetrical key pair.
		encryptedSymmetricKey->CopyFrom(symmetricKey);
		encryptedSymmetricKey->PowMod(publicKey, n);

		// compress our data and make the size a multiple of the (block size + 2*8) bytes for the size information. The block size is usually given in bits for cryptography.
		if (!CompressData(origData, origDataSize, compressedData, compressedDataSize, 9) || !compressedData || !compressedDataSize)
			return false;
		j = blockSize / 8;
		encryptedSize = (compressedDataSize + 2 * 8 + j - 1) & ~(j - 1);
		transmit = NewMem(maxon::UChar, encryptedSize);
		if (!transmit)
		{
			DeleteMem(compressedData);
			return false;
		}
		*((maxon::UInt64*)(transmit + 0)) = (maxon::UInt64)origDataSize;
		*((maxon::UInt64*)(transmit + 8)) = (maxon::UInt64)compressedDataSize;
		CopyMem(compressedData, transmit + 16, compressedDataSize);
		j = 16 + compressedDataSize;
		// fill the memory up to the block boundary with random data to increase security.
		if (j < encryptedSize)
			SecureRandom::GetRandomNumber(provider, transmit + j, encryptedSize - j);
		DeleteMem(compressedData);

		// now encrypt everything with the symmetrical key
		ClearMem(key, sizeof(key));
		symmetricKey->GetDataCopy(keyArray);
		DebugAssert(keyArray.GetCount() <= keySize / 8);
		CopyMem(keyArray.GetFirst(), key, maxon::Min(keyArray.GetCount(), keySize / 8));
		crypt->Init(blockSize, keySize);
		crypt->Encrypt(transmit, encryptedSize, key);

		transmitData = transmit;
		transmitDataSize = encryptedSize;

		return true;
	}

	Bool DecryptData(maxon::UChar* transmitData, maxon::Int transmitDataSize, const BigInteger* privateKey, const BigInteger* n, const BigInteger* encryptedSymmetricKey, maxon::UChar*& receiveData, maxon::Int& receiveDataSize)
	{
		maxon::UChar				key[blockSize / 8];
		AutoAlloc<BigInteger> symmetricKey;
		maxon::BaseArray<maxon::UChar> keyArray;
		AutoAlloc<AES>	 crypt;
		maxon::UInt64 compressedDataSize;
		void* uncompressedData = nullptr;
		Int		uncompressedDataSize = 0;

		if (!symmetricKey || !crypt)
			return false;

		// decrypt the symmetric key
		symmetricKey->CopyFrom(encryptedSymmetricKey);
		symmetricKey->PowMod(privateKey, n);

		// decrypt the data using the symmetric key
		ClearMem(key, sizeof(key));
		symmetricKey->GetDataCopy(keyArray);
		DebugAssert(keyArray.GetCount() <= keySize / 8);
		CopyMem(keyArray.GetFirst(), key, maxon::Min(keyArray.GetCount(), keySize / 8));
		crypt->Init(blockSize, keySize);
		crypt->Decrypt(transmitData, transmitDataSize, key);

		// extract size information
		//origBufferSize = *((UInt64*)(transmitData + 0));
		compressedDataSize = *((maxon::UInt64*)(transmitData + 8));

		// uncompress
		if (!UncompressData(transmitData + 16, (Int)compressedDataSize, uncompressedData, uncompressedDataSize) || !uncompressedData || !uncompressedDataSize)
			return false;

		receiveData = (maxon::UChar*)uncompressedData;
		receiveDataSize = uncompressedDataSize;

		return true;
	}

	Bool CreateSignature(const void* data, maxon::Int dataSize, const BigInteger* privateKeySender, const BigInteger* nSender, BigInteger* signature)
	{
		// The signature is a hash value of the data that is encrypted with the sender's private key. We take the CRC value here for simplicity.
		UInt32 crc = ZipFile::CalcCRC32(data, (UInt32)dataSize);
		signature->Set((maxon::UInt32)crc);
		signature->PowMod(privateKeySender, nSender);

		return true;
	}

	Bool CheckSignature(const void* data, maxon::Int dataSize, const BigInteger* publicKeySender, const BigInteger* nSender, const BigInteger* signature, maxon::Bool& signatureOK)
	{
		// The signature is a hash value of the data that is encrypted with the sender's private key. We take the CRC value here for simplicity.
		AutoAlloc<BigInteger> signatureTest, hash;
		if (!signatureTest || !hash)
			return false;
		signatureTest->CopyFrom(signature);
		UInt32 crc = ZipFile::CalcCRC32(data, (UInt32)dataSize);
		hash->Set((maxon::UInt32)crc);

		signatureTest->PowMod(publicKeySender, nSender);
		signatureOK = signatureTest->Compare(hash) == maxon::COMPARERESULT_EQUAL;

		return true;
	}
};

Bool RegisterPGPTest()
{
	// be sure to use a unique ID obtained from www.plugincafe.com
	return RegisterCommandPlugin(450000266, GeLoadString(IDS_PGPTEST), 0, nullptr, String("C++ SDK PGP test"), NewObjClear(PGPTest));
}
