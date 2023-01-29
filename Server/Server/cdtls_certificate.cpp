﻿/***********************************************************************************************
created: 		2023-01-18

author:			chensong

purpose:		_C_DTLS_CERTIFICATE_H_


************************************************************************************************/
#include "cdtls_certificate.h"
#include "crandom.h"
namespace chen {


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#pragma GCC diagnostic pop

	cdtls_certificate   g_dtls_certificate;
	cdtls_certificate::~cdtls_certificate()
	{
	}
	bool cdtls_certificate::init()
	{

		int32 ret = 0;
		X509_NAME * cert_name_ptr = NULL;
		std::string subject = std::string("cmedia_rtc_server") + std::to_string(c_rand.rand(100000, 999999));


		// 1. 使用曲线创建关键点。
		m_eckey_ptr = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
		if (!m_eckey_ptr)
		{
			WARNING_EX_LOG("EC_KEY_new_by_curve_name() failed");
			return false;
		}


		EC_KEY_set_asn1_flag(m_eckey_ptr, OPENSSL_EC_NAMED_CURVE);

		// NOTE: This can take some time.
		ret = EC_KEY_generate_key(m_eckey_ptr);
		if (ret == 0)
		{
			WARNING_EX_LOG("EC_KEY_generate_key() failed"); 
			return false;
		}

		// 2. 创建私钥对象。
		m_private_key_ptr = EVP_PKEY_new();

		if (!m_private_key_ptr)
		{
			WARNING_EX_LOG("EVP_PKEY_new() failed");

			return false;
		}
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
		ret = EVP_PKEY_assign_EC_KEY(m_private_key_ptr, m_eckey_ptr);

		if (ret == 0)
		{
			WARNING_EX_LOG("EVP_PKEY_assign_EC_KEY() failed");
			return false;
		}


		//////////////////////////////////////////////////////////////////////////

		// The EC key now belongs to the private key, so don't clean it up separately.
		// m_eckey_ptr = nullptr;
		//////////////////////////////////////////////////////////////////////////


		// 3. 创建 X509 证书.
		m_certificate_ptr = X509_new();

		if (!m_certificate_ptr)
		{
			ERROR_EX_LOG("X509_new() failed");
			return false;
		}

		// Set version 3 (note that 0 means version 1). 
		X509_set_version(m_certificate_ptr, 2);

		// Set serial number (avoid default 0).
		ASN1_INTEGER_set( X509_get_serialNumber(m_certificate_ptr), static_cast<uint64_t>(9999999));

		// Set valid period.
		X509_gmtime_adj(X509_get_notBefore(m_certificate_ptr), -315360000); // -10 years.
		X509_gmtime_adj(X509_get_notAfter(m_certificate_ptr), 315360000);   // 10 years.

		// Set the public key for the certificate using the key.
		ret = X509_set_pubkey(m_certificate_ptr, m_private_key_ptr);

		if (ret == 0)
		{
			WARNING_EX_LOG("X509_set_pubkey() failed");
			return false;
		}

		// Set certificate fields.
		 cert_name_ptr = X509_get_subject_name(m_certificate_ptr);

		if (!cert_name_ptr)
		{
			WARNING_EX_LOG("X509_get_subject_name() failed");

			return false;
		}

		X509_NAME_add_entry_by_txt(
			cert_name_ptr, "O", MBSTRING_ASC, reinterpret_cast<const uint8_t*>(subject.c_str()), -1, -1, 0);
		X509_NAME_add_entry_by_txt(
			cert_name_ptr, "CN", MBSTRING_ASC, reinterpret_cast<const uint8_t*>(subject.c_str()), -1, -1, 0);

		// It is self-signed so set the issuer name to be the same as the subject.
		ret = X509_set_issuer_name(m_certificate_ptr, cert_name_ptr);

		if (ret == 0)
		{
			WARNING_EX_LOG("X509_set_issuer_name() failed");
			X509_NAME_free(cert_name_ptr);
			cert_name_ptr = NULL;
			return false;
		}

		// Sign the certificate with its own private key.
		ret = X509_sign(m_certificate_ptr  , m_private_key_ptr, EVP_sha1());

		if (ret == 0)
		{
			WARNING_EX_LOG("X509_sign() failed");
			X509_NAME_free(cert_name_ptr);
			cert_name_ptr = NULL;
			return false;
		}
		X509_NAME_free(cert_name_ptr);
		cert_name_ptr = NULL;



		// Show DTLS fingerprint
		if (true) 
		{
			char fp[100] = { 0 };
			char *p = fp;
			unsigned char md[EVP_MAX_MD_SIZE];
			unsigned int n = 0;

			// TODO: FIXME: Unused variable.
			/*int r = */X509_digest(m_certificate_ptr, EVP_sha256(), md, &n);

			for (unsigned int i = 0; i < n; i++, ++p)
			{
				sprintf(p, "%02X", md[i]);
				p += 2;

				if (i < (n - 1)) 
				{
					*p = ':';
				}
				else 
				{
					*p = '\0';
				}
			}

			m_fingerprints .assign(fp, strlen(fp));
			NORMAL_EX_LOG("fingerprint=%s", m_fingerprints.c_str());
		}


		return true;
	}
	void cdtls_certificate::destroy()
	{
		if (m_eckey_ptr)
		{
			EC_KEY_free(m_eckey_ptr);
			m_eckey_ptr = NULL;
		}

		if (m_private_key_ptr)
		{
			EVP_PKEY_free(m_private_key_ptr);
			m_private_key_ptr = NULL;
		}
		 
		if (m_certificate_ptr)
		{
			X509_free(m_certificate_ptr);
			m_certificate_ptr = NULL;
		}
		m_fingerprints.clear();
	}
	X509 * cdtls_certificate::get_cert()
	{
		return m_certificate_ptr;
	}
	EVP_PKEY * cdtls_certificate::get_public_key()
	{
		return m_private_key_ptr;
	}
	EC_KEY * cdtls_certificate::get_ecdsa_key()
	{
		return m_eckey_ptr;
	}
	std::string cdtls_certificate::get_fingerprint()
	{
		return m_fingerprints;
	}
}