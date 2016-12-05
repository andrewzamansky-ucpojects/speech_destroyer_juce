#ifndef CONTROLBASECOMPONENT_H_INCLUDED
#define CONTROLBASECOMPONENT_H_INCLUDED

class ControlBaseComponent :
	public ChangeBroadcaster
{
public:
	ControlBaseComponent()	{	}
	virtual void sendCommand(const char* cmd) = 0;
	virtual void getEvent(String& event) = 0;
	//~MainContentComponent() {	}

};

#endif
