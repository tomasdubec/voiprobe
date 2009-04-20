#include "db.h"

bool __packet::operator<(const __packet &p)const{
	if(realtime < p.realtime)
		return true;
	else
		return false;
}

DB::DB(string f){
	req2soon = 0;
}

DB::~DB(){

}

bool DB::insertOutgoingPacket(int seqn, int timestamp, int realtime, int ssrc){
	__packet p;

	p.seqid = seqn;
	p.timestamp = timestamp;
	p.realtime = realtime;
	p.ssrc = ssrc;
	p.gotAnswer = false;

	outgoingPackets.push_front(p);

	while(outgoingPackets.size() > 300){
		outgoingPackets.pop_back();
	}

	return true;
}

bool DB::insertIncomingPacket(int seqn, int timestamp, int realtime, int ssrc){
	__packet p;

	p.seqid = seqn;
	p.timestamp = timestamp;
	p.realtime = realtime;
	p.ssrc = ssrc;
	p.gotAnswer = false;

	incomingPackets.push_back(p);

	while(incomingPackets.size() > 300){
		incomingPackets.pop_front();
	}

	return true;
}

bool DB::getLatestOutgoingPacket(int &sid, int &ts, int &rt, int &ss){
	list<__packet>::iterator it;
	
	if(outgoingPackets.empty())
		return false;

	/*outgoingPackets.sort();
	outgoingPackets.reverse();*/

	if(outgoingPackets.size() < 6)
		return false;

	it = outgoingPackets.begin();
	it++;it++;it++;it++;

	if(!it->gotAnswer){
		sid = it->seqid;
		ts = it->timestamp;
		rt = it->realtime;
		ss = it->ssrc;
		return true;
	}
	else //latest packet was already used for latency computation
		return false;

	//TODO deletePacket method

}

/*bool DB::getClosest(int id, int &fid, int32_t &ts){
	__packet last, main;
	list<__packet>::iterator it;

	if(outgoingPackets.empty())
		return false;

	last.seqid = -1;
	main.seqid = id;
	if(!getIncomingPacket(id, main.timestamp, main.realtime, main.ssrc))
		return false;

	outgoingPackets.sort();
	outgoingPackets.sort();

	for(it = outgoingPackets.begin(); it != outgoingPackets.end(); it++){
		cout << "********************************************************** main.realtime: " << main.realtime << ", it->realtime: " << it->realtime << endl;
		if(main < (*it) && it != outgoingPackets.begin()){
			if((main.realtime - last.realtime) > (it->realtime - main.realtime)){
				fid = it->seqid;
				ts = main.realtime - it->realtime;
				cout << "************************************************************** vracim vetsi\n";
			}
			else{
				fid = last.seqid;
				ts = main.realtime - last.realtime;
				cout << "************************************************************** vracim mensi\n";
			}
			return true;
		}
		else if(main < (*it) && it == outgoingPackets.begin()){
			fid = it->seqid;
			ts = main.realtime - it->realtime;
			cout << "************************************************************** vracim vetsi (mensi neexistuje)\n";
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
	cout << "************************************************************** vracim mensi " << fid << " (vetsi neexistuje)\n";
	
	return true;
}*/

bool DB::getClosest(int id, int &fid, int32_t &ts){
	__packet last, main;
	list<__packet>::iterator it;


	if(outgoingPackets.empty())
		return false;

	last.seqid = -1;
	main.seqid = id;
	if(!getIncomingPacket(id, main.timestamp, main.realtime, main.ssrc)){ //this request was faster then the actual packet, which we don't yet have
		req2soon++;
		//cerr << "request too soon! (" << req2soon << ")\n";
		return false;
	}

	/*outgoingPackets.sort();
	outgoingPackets.reverse();*/

	for(it = outgoingPackets.begin(); it != outgoingPackets.end(); it++){
		if(it->realtime < main.realtime){ 	
			if(last.seqid == -1){ //latest packet is already older, no need to look any further
				fid = it->seqid;
				ts = main.realtime - it->realtime;
				return true;
			}
			else{ //found first packet older (but also some younger from before)
				if((last.realtime - main.realtime) < (main.realtime - it->realtime)){ //find out, which of those two is closer
					fid = last.seqid;
					ts = main.realtime - last.realtime;
					return true;
				}
				else{
					fid = it->seqid;
					ts = main.realtime - it->realtime;
					return true;
				}
			}
		}
		else{
			last.seqid = it->seqid;
			last.timestamp = it->timestamp;
			last.realtime = it->realtime;
			last.ssrc = it->ssrc;
			last.gotAnswer = it->gotAnswer;
		}
	}
	//we didn't find older, but list is not empty, so 'last' contains closest packet
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

bool DB::markGotAnswer(int seqid){
	list<__packet>::iterator it;
	
	//return true;

	for(it = outgoingPackets.begin(); it != outgoingPackets.end(); it++){
		if(it->seqid == seqid){
			it->gotAnswer = true;
			return true;
		}
	}
	return false;
}
