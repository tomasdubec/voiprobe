#include "db.h"

bool __packet::operator<(const __packet &p)const{
	if(realtime < p.realtime)
		return true;
	else
		return false;
}

DB::DB(string f){

}

DB::~DB(){

}

bool DB::insertOutgoingPacket(int seqn, int timestamp, int realtime, int ssrc){
	__packet p;

	p.seqid = seqn;
	p.timestamp = timestamp;
	p.realtime = realtime;
	p.ssrc = ssrc;

	outgoingPackets.push_back(p);

	return true;
}

bool DB::insertIncomingPacket(int seqn, int timestamp, int realtime, int ssrc){
	__packet p;

	p.seqid = seqn;
	p.timestamp = timestamp;
	p.realtime = realtime;
	p.ssrc = ssrc;

	incomingPackets.push_back(p);

	return true;
}

bool DB::getLatestOutgoingPacket(int &sid, int &ts, int &rt, int &ss){
	list<__packet>::iterator it;
	
	if(outgoingPackets.empty())
		return false;

	outgoingPackets.sort();
	outgoingPackets.reverse();

	it = outgoingPackets.begin();
	sid = it->seqid;
	ts = it->timestamp;
	rt = it->realtime;
	ss = it->ssrc;

	//TODO deletePacket method

	return true;
}

bool DB::getClosest(int id, int &fid, int32_t &ts){
	__packet last, main;
	list<__packet>::iterator it;

	if(outgoingPackets.empty())
		return false;

	last.seqid = -1;
	main.seqid = id;
	if(!getIncomingPacket(id, main.timestamp, main.realtime, main.ssrc))
		return false;

	incomingPackets.sort();

	for(it = outgoingPackets.begin(); it != outgoingPackets.end(); it++){
		if(main < (*it) && it != outgoingPackets.begin()){
			if((main.realtime - last.realtime) > (it->realtime - main.realtime)){
				fid = it->seqid;
				ts = main.realtime - it->realtime;
			}
			else{
				fid = last.seqid;
				ts = main.realtime - last.realtime;
			}
			return true;
		}
		else if(main < (*it) && it == outgoingPackets.begin()){
			fid = it->seqid;
			ts = main.realtime - it->realtime;
			return true;
		}
		else{
			last.seqid = it->seqid;
			last.timestamp = it->timestamp;
			last.realtime = it->realtime;
			last.ssrc = it->ssrc;
		}
	}
	//we didn't find bigger, but list is not empty, so last contains closest packet
	fid = last.seqid;
	ts = main.realtime - last.realtime;
	
	return true;
}

bool DB::getIncomingPacket(int sid, int &ts, int &rt, int &ss){
	list<__packet>::iterator it;

	if(incomingPackets.empty())
		return false;

	for(it = incomingPackets.begin(); it != incomingPackets.end(); it++){
		if(it->seqid == sid){
			ts = it->timestamp;
			rt = it->realtime;
			ss = it->ssrc;
			return true;
		}
	}

	return false;
}


