#ifndef CONTROLBASECOMPONENT_H_INCLUDED
#define CONTROLBASECOMPONENT_H_INCLUDED

class ControlBaseComponent
{
public:
	ControlBaseComponent()	{	}
	virtual void sendCommand(const char* cmd) = 0;
	//~MainContentComponent() {	}

};

#endif
