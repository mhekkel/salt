//           Copyright Maarten L. Hekkelman 2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <assh/config.hpp>

#include <iostream>

#include <boost/algorithm/string/finder.hpp>
#include <boost/algorithm/string/find_iterator.hpp>

#include <cryptopp/gfpcrypt.h>
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/modes.h>

#include <assh/key_exchange.hpp>
#include <assh/error.hpp>
#include <assh/connection.hpp>

using namespace std;
using namespace CryptoPP;
namespace ba = boost::algorithm;

namespace assh
{

static AutoSeededRandomPool	rng;

// --------------------------------------------------------------------
// a utility class to make hashing easier
	
template<typename HAlg>
class hash
{
  public:
						hash() {}

	hash&				update(const CryptoPP::Integer& v)
						{
							opacket p;
							p << v;
							return update(static_cast<std::vector<uint8>>(p));
						}

	hash&				update(const std::vector<uint8>& v)
						{
							m_hash.Update(&v[0], v.size());
							return *this;
						}
						
	hash&				update(const std::string& v)
						{
							m_hash.Update(reinterpret_cast<const uint8*>(v.c_str()), v.length());
							return *this;
						}
						
	hash&				update(const char* v)
						{
							m_hash.Update(v, std::strlen(v));
							return *this;
						}
						
	hash&				update(uint8 v)
						{
							m_hash.Update(&v, 1);
							return *this;
						}
	
	std::vector<uint8>	final()
						{
							std::vector<uint8> result(m_hash.DigestSize());
							m_hash.Final(&result[0]);
							return result;
						}

  private:
						hash(const hash&);
	hash&				operator=(const hash&);

	HAlg				m_hash;
};

template<typename H, typename T>
hash<H>& operator|(hash<H>& h, T t)
{
	return h.update(t);
}

// --------------------------------------------------------------------
	
key_exchange::key_exchange(const string& host_version, vector<uint8>& session_id,
	const vector<uint8>& my_payload, const vector<uint8>& host_payload)
	: m_session_id(session_id), m_host_version(host_version)
	, m_host_payload(host_payload), m_my_payload(my_payload)
{
}

bool key_exchange::process(ipacket& in, opacket& out, boost::system::error_code& ec)
{
	bool handled = true;
	
	switch ((message_type)in)
	{
		case msg_kex_dh_reply:
			process_kex_dh_reply(in, out, ec);
			break;
		
		default:
			handled = false;
	}
	
	return handled;
}

void key_exchange::process_kex_dh_reply(ipacket& in, opacket& out, boost::system::error_code& ec)
{
	ipacket hostkey, signature;
	Integer f;

	in >> hostkey >> f >> signature;
	
	m_K = a_exp_b_mod_c(f, m_x, m_p);

	calculate_hash(hostkey, f);

	if (m_session_id.empty())
		m_session_id = m_H;

	unique_ptr<PK_Verifier> h_key;

	string pk_type;
	ipacket pk_rs;
	signature >> pk_type >> pk_rs;

	if (cb_verify_host_key and not cb_verify_host_key(pk_type, hostkey))
		ec = error::make_error_code(error::host_key_verification_failed);
	else
	{
		string h_pk_type;
		hostkey >> h_pk_type;
	
		if (h_pk_type == "ssh-dss")
		{
			Integer h_p, h_q, h_g, h_y;
			hostkey >> h_p >> h_q >> h_g >> h_y;
	
			h_key.reset(new GDSA<SHA1>::Verifier(h_p, h_q, h_g, h_y));
		}
		else if (h_pk_type == "ssh-rsa")
		{
			Integer h_e, h_n;
			hostkey >> h_e >> h_n;
	
			h_key.reset(new RSASSA_PKCS1v15_SHA_Verifier(h_n, h_e));
		}
	
		vector<uint8> pk_rs_d = pk_rs;
		if (pk_type != h_pk_type or not h_key->VerifyMessage(&m_H[0], m_H.size(), &pk_rs_d[0], pk_rs_d.size()))
			ec = error::make_error_code(error::host_key_verification_failed);
		else
			out = msg_newkeys;
	}

	derive_keys_with_hash();		// to avoid problems
}

void key_exchange::derive_keys_with_hash()
{
	derive_keys<SHA1>();
}

template<typename HashAlgorithm>
void key_exchange::derive_keys()
{
	// derive the keys, 64 bytes should be enough
	int keylen = 64;
	for (int i = 0; i < 6; ++i)
	{
		hash<HashAlgorithm> ha;
		vector<uint8> key = (ha | m_K | m_H | ('A' + i) | m_session_id).final();
		
		for (int k = 20; k < keylen; k += 20)
		{
			vector<uint8> k2 = (ha | m_K | m_H | key).final();
			key.insert(key.end(), k2.begin(), k2.end());
		}
		
		m_keys[i].assign(key.begin(), key.begin() + keylen);
	}
}

// --------------------------------------------------------------------
	
class key_exchange_dh_group : public key_exchange
{
  public:
							key_exchange_dh_group(const Integer& p,
								const string& host_version, vector<uint8>& session_id,
								const vector<uint8>& my_payload, const vector<uint8>& host_payload)
								: key_exchange(host_version, session_id, my_payload, host_payload)
							{
								m_p = p;
								m_q = (p - 1) / 2;
								m_g = 2;
							}

	virtual bool			process(ipacket& in, opacket& out, boost::system::error_code& ec);
	virtual void			calculate_hash(ipacket& hostkey, Integer& f);
};

bool key_exchange_dh_group::process(ipacket& in, opacket& out, boost::system::error_code& ec)
{
	bool handled = true;
	
	switch ((message_type)in)
	{
		case msg_kexinit:
			do
			{
				m_x.Randomize(rng, m_g, m_q - 1);
				m_e = a_exp_b_mod_c(m_g, m_x, m_p);
			}
			while (m_e < 1 or m_e >= m_p - 1);
			
			out = msg_kex_dh_init;
			out << m_e;
			break;

		default:
			handled = key_exchange::process(in, out, ec);
			break;
	}
	
	return handled;
}

void key_exchange_dh_group::calculate_hash(ipacket& hostkey, Integer& f)
{
	opacket hp;
	hp << kSSHVersionString << m_host_version << m_my_payload << m_host_payload << hostkey << m_e << f << m_K;

	m_H = hash<SHA1>().update(hp).final();
}

// --------------------------------------------------------------------

template<typename HashAlgorithm>
class key_exchange_dh_gex : public key_exchange
{
  public:
							key_exchange_dh_gex(const string& host_version, vector<uint8>& session_id,
								const vector<uint8>& my_payload, const vector<uint8>& host_payload)
								: key_exchange(host_version, session_id, my_payload, host_payload)
							{
							}

	virtual bool			process(ipacket& in, opacket& out, boost::system::error_code& ec);
	virtual void			calculate_hash(ipacket& hostkey, Integer& f);
	virtual void			derive_keys_with_hash();

	static const uint32		kMinGroupSize = 1024,
							kPreferredGroupSize = 2048,
							kMaxGroupSize = 8192;
};

template<typename HashAlgorithm>
bool key_exchange_dh_gex<HashAlgorithm>::process(ipacket& in, opacket& out, boost::system::error_code& ec)
{
	bool handled = true;
	
	switch ((message_type)in)
	{
		case msg_kexinit:
			out = msg_kex_dh_gex_request;
			out << kMinGroupSize << kPreferredGroupSize << kMaxGroupSize;
			break;

		case msg_kex_dh_gex_group:
			in >> m_p >> m_g;
			m_q = (m_p - 1) / 2;
			
			do
			{
				m_x.Randomize(rng, m_g, m_q - 1);
				m_e = a_exp_b_mod_c(m_g, m_x, m_p);
			}
			while (m_e < 1 or m_e >= m_p - 1);
			
			out = msg_kex_dh_gex_init;
			out << m_e;
			break;

		case msg_kex_dh_gex_reply:		
			process_kex_dh_reply(in, out, ec);
			break;

		default:
			handled = key_exchange::process(in, out, ec);
			break;
	}
	
	return handled;
}

template<typename HashAlgorithm>
void key_exchange_dh_gex<HashAlgorithm>::calculate_hash(ipacket& hostkey, Integer& f)
{
	opacket hp;

	hp	<< kSSHVersionString << m_host_version << m_my_payload << m_host_payload << hostkey
		<< kMinGroupSize << kPreferredGroupSize << kMaxGroupSize
		<< m_p << m_g << m_e << f << m_K;

	m_H = hash<HashAlgorithm>().update(hp).final();
}

template<typename HashAlgorithm>
void key_exchange_dh_gex<HashAlgorithm>::derive_keys_with_hash()
{
	derive_keys<HashAlgorithm>();
}

key_exchange* key_exchange::create(const string& key_exchange_alg, const string& host_version,
	vector<uint8>& session_id, const vector<uint8>& host_payload, const vector<uint8>& my_payload)
{
	// diffie hellman group 1 and group 14 primes
	const unsigned char
		p2[] = {
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
			0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1, 0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
			0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22, 0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
			0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B, 0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
			0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45, 0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
			0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B, 0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
			0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5, 0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
			0x49, 0x28, 0x66, 0x51, 0xEC, 0xE6, 0x53, 0x81, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
		},
		p14[] = {
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
			0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1, 0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
			0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22, 0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
			0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B, 0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
			0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45, 0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
			0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B, 0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
			0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5, 0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
			0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D, 0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05,
			0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A, 0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
			0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96, 0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB,
			0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D, 0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04,
			0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x18, 0x21, 0x7C, 0x32, 0x90, 0x5E, 0x46, 0x2E, 0x36, 0xCE, 0x3B,
			0xE3, 0x9E, 0x77, 0x2C, 0x18, 0x0E, 0x86, 0x03, 0x9B, 0x27, 0x83, 0xA2, 0xEC, 0x07, 0xA2, 0x8F,
			0xB5, 0xC5, 0x5D, 0xF0, 0x6F, 0x4C, 0x52, 0xC9, 0xDE, 0x2B, 0xCB, 0xF6, 0x95, 0x58, 0x17, 0x18,
			0x39, 0x95, 0x49, 0x7C, 0xEA, 0x95, 0x6A, 0xE5, 0x15, 0xD2, 0x26, 0x18, 0x98, 0xFA, 0x05, 0x10,
			0x15, 0x72, 0x8E, 0x5A, 0x8A, 0xAC, 0xAA, 0x68, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
		};

	key_exchange* result = nullptr;
	
	if (key_exchange_alg == "diffie-hellman-group1-sha1")
		result = new key_exchange_dh_group(Integer(p2, sizeof(p2)), host_version,
			session_id, my_payload, host_payload);
	else if (key_exchange_alg == "diffie-hellman-group14-sha1")
		result = new key_exchange_dh_group(Integer(p14, sizeof(p14)), host_version,
			session_id, my_payload, host_payload);
	else if (key_exchange_alg == "diffie-hellman-group-exchange-sha1")
		result = new key_exchange_dh_gex<SHA1>(host_version, session_id, my_payload, host_payload);
	else if (key_exchange_alg == "diffie-hellman-group-exchange-sha256")
		result = new key_exchange_dh_gex<SHA256>(host_version, session_id, my_payload, host_payload);
	
	return result;
}

}
