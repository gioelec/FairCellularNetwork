#include "User.h"
int User::USER_COUNTER=0;
Define_Module(User);

void User::initialize()
{
    scheduleAt(simTime()+par("timeSlotPeriod"),timeSlotTimer);
    resp_signal = registerSignal("resp");
}

int User::computeCqi() {
    int cqi;
    int how = par("uniformServ");
    if(how==0) //Uniform case
        cqi = uniform(1,15);
    else if(how==1){ //Binomial case with p proportional to UID
        int n = par("n");
        cqi = binomial(14, (double)(userID+1)/(n+1),par("CqiRNGID"))+1;
    }else{ //Binomial case with only two kinds of p, one for odd UID the other for even ones
        int division = userID%2;
        double p = division*0.5+0.25;
        cqi = binomial(14, p,par("CqiRNGID"))+1;
    }
    return cqi;
}

void User::handleFrame(Frame* frame) {
    EV << "Received a frame" << endl;
    long previousMsgId = -1;
    bool alreadyDone = false;
    //Scan the broadcast Frame
    for(int i=0; i<25; i++){
        //Check if the RB contains a packet that belongs to me.
        ResourceBlock* rb = frame->get_rbs(i);
        if(!rb->getSize()) continue;
        if(rb->getUserID() != userID && alreadyDone) break;
        if(rb->getUserID() != userID) continue;
        //EV << "ResourceBlock -- " << i << " belongs to me" << endl;
        alreadyDone = true;
        Packet* p = NULL;
        while(!(rb->isEmpty())){
           p = rb->popPacket();
           if(previousMsgId == -1 || previousMsgId != p->getTreeId()) {
               EV << "Extracted packet -- " << p->getTreeId() << " size -- " << p->getSize() << endl;
               simtime_t responseTime = simTime() - p->getCreation();
               EV << responseTime << endl;
               emit(resp_signal,responseTime);
           }
           if(p->getFragment()) {
               //EV << "\tPacket -- " << p->getTreeId() << " is a fragment of -- " << p->getId()  << endl;
               previousMsgId = p->getTreeId();
           }
           else
               previousMsgId = -1;
           delete p;
         }
     }
     delete frame;
}

void User::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage()) {
    	//send CQI to the Antenna 
        Cqi* cqiMsg = new Cqi();
        cqiMsg->setCqiValue(computeCqi());
        cqiMsg->setUserID(userID);
        cqiMsg->setSchedulingPriority(1111);
        cqiMsg->setName("CQI");
        send(cqiMsg,"out");
        scheduleAt(simTime()+par("timeSlotPeriod"),timeSlotTimer);
    }
    else{
        Frame* frame = check_and_cast<Frame *>(msg);
    	handleFrame(frame);
    }
}
