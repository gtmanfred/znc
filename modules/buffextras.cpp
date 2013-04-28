/*
 * Copyright (C) 2004-2013 ZNC, see the NOTICE file for details.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <znc/Chan.h>
#include <znc/IRCNetwork.h>

using std::vector;

class CBuffExtras : public CModule {
public:
	MODCONSTRUCTOR(CBuffExtras) {}

	virtual ~CBuffExtras() {}

	bool WantAdd(CChan& Channel) {
		// If they have AutoClearChanBuffer enabled, only add messages if no client is connected
		return !(Channel.AutoClearChanBuffer() && m_pNetwork->IsUserOnline());
	}

#if 0
	// TODO: use this conditionally, if server-time is off for this client
	void AddBuffer(CChan& Channel, const CString& sMessage) {
		if (!WantAdd(Channel))
			return;
		Channel.AddBuffer(":" + GetModNick() + "!" + GetModName() + "@znc.in PRIVMSG " + _NAMEDFMT(Channel.GetName()) + " :{text}", sMessage);
	}
#endif

	virtual void OnRawMode(const CNick& OpNick, CChan& Channel, const CString& sModes, const CString& sArgs) {
		if (!WantAdd(Channel))
			return;
		Channel.AddBuffer(":" + _NAMEDFMT(OpNick.GetNickMask()) + " MODE " + _NAMEDFMT(Channel.GetName()) + " " + sModes + " " + _NAMEDFMT(sArgs));
	}

	virtual void OnKick(const CNick& OpNick, const CString& sKickedNick, CChan& Channel, const CString& sMessage) {
		if (!WantAdd(Channel))
			return;
		Channel.AddBuffer(":" + _NAMEDFMT(OpNick.GetNickMask()) + " KICK " + _NAMEDFMT(Channel.GetName()) + " " + _NAMEDFMT(sKickedNick) + " :{text}", sMessage);
	}

	virtual void OnQuit(const CNick& Nick, const CString& sMessage, const vector<CChan*>& vChans) {
		vector<CChan*>::const_iterator it;
		CString sMsg = ":" + _NAMEDFMT(Nick.GetNickMask()) + " QUIT :" + sMessage;
		for (it = vChans.begin(); it != vChans.end(); ++it) {
			(**it).AddBuffer(sMsg);
		}
	}

	virtual void OnJoin(const CNick& Nick, CChan& Channel) {
		if (!WantAdd(Channel))
			return;
		Channel.AddBuffer(":" + _NAMEDFMT(Nick.GetNickMask()) + " JOIN " + _NAMEDFMT(Channel.GetName()));
	}

	virtual void OnPart(const CNick& Nick, CChan& Channel, const CString& sMessage) {
		if (!WantAdd(Channel))
			return;
		Channel.AddBuffer(":" + _NAMEDFMT(Nick.GetNickMask()) + " PART " + _NAMEDFMT(Channel.GetName()) + " :{text}", sMessage);
	}

	virtual void OnNick(const CNick& OldNick, const CString& sNewNick, const vector<CChan*>& vChans) {
		vector<CChan*>::const_iterator it;
		CString sMsg = ":" + _NAMEDFMT(OldNick.GetNickMask()) + " NICK " + _NAMEDFMT(sNewNick);
		for (it = vChans.begin(); it != vChans.end(); ++it) {
			(**it).AddBuffer(sMsg);
		}
	}

	virtual EModRet OnTopic(CNick& Nick, CChan& Channel, CString& sTopic) {
		if (!WantAdd(Channel))
			return CONTINUE;
		Channel.AddBuffer(":" + _NAMEDFMT(Nick.GetNickMask()) + " TOPIC " + _NAMEDFMT(Channel.GetName()) + " :" + _NAMEDFMT(sTopic));
		return CONTINUE;
	}
};

template<> void TModInfo<CBuffExtras>(CModInfo& Info) {
	Info.SetWikiPage("buffextras");
}

USERMODULEDEFS(CBuffExtras, "Add joins, parts etc. to the playback buffer")

