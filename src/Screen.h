#ifndef ISCREEN_H_
#define ISCREEN_H_

// Interface.
class IScreen
{
protected:
	IScreen *Parent;
	double ScreenTime; // How long has it been open?
	bool Running; // Is this screen active?

public:

	IScreen *Next;

	IScreen ();
	IScreen (IScreen * _Parent);
	virtual ~IScreen ();

	// Nesting screens.
	bool IsScreenRunning();
	bool RunNested(float delta);

	// Screen implementation.
	virtual void LoadThreadInitialization();
	virtual void MainThreadInitialization();
	virtual bool Run(double delta) = 0;
	virtual void HandleInput(int32 key, KeyEventType state, bool isMouseInput);

	// We need to set up graphics again? This gets called.
	virtual void Invalidate();

	// Implement this if there's anything you want to get done outside of a destructor
	// like operations that would throw exceptions.
	virtual void Cleanup();

};

#endif