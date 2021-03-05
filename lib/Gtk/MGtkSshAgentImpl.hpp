//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

class MSshPacket;

class MGtkSshAgentImpl : public MSshAgentImpl
{
  public:
	static MSshAgent*	Create();

	bool				GetFirstIdentity(CryptoPP::Integer& e, CryptoPP::Integer& n, std::string& outComment);
	
	bool				GetNextIdentity(CryptoPP::Integer& e, CryptoPP::Integer& n, std::string& outComment);
	
	void				SignData(const std::string& inBlob, const std::string& inData, std::string& outSignature);
	
  public:
						MSshAgent(int inSock);

						~MSshAgent();	

	bool				RequestReply(MSshPacket& out, MSshPacket& in);

	int					mSock;
	MSshPacket			mIdentities;
	uint32_t				mCount;
	CryptoPP::Integer	e, n;
};
