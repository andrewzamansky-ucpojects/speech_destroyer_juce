/*
  ==============================================================================

	This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MainComponent.h"

//==============================================================================
class NuAEC_RNNApplication  : public juce::JUCEApplication
{
public:
	//=========================================================================
	NuAEC_RNNApplication() {}

	const juce::String getApplicationName() override
			{ return ProjectInfo::projectName; }
	const juce::String getApplicationVersion() override
			{ return ProjectInfo::versionString; }
	bool moreThanOneInstanceAllowed() override
			{ return true; }

	//=========================================================================
	void initialise (const juce::String& commandLine) override
	{
		// This is where you should put your application's initialisation code..

		mainWindow.reset (new MainWindow (getApplicationName()));
	}

	void shutdown() override
	{
		// Add your application's shutdown code here..

		mainWindow = nullptr; // (deletes our window)
	}

	//========================================================================
	void systemRequestedQuit() override
	{
		// This is called when the app is being asked to quit: you can ignore
		// this request and let the app carry on running, or call quit()
		// to allow the app to close.
		quit();
	}

	void anotherInstanceStarted (const juce::String& commandLine) override
	{
		// When another instance of the app is launched while this one is
		// running, this method is invoked, and the commandLine parameter tells
		// you what the other instance's command-line arguments were.
	}

	//========================================================================
	/*
		This class implements the desktop window that contains an instance of
		our MainComponent class.
	*/
	class MainWindow : public juce::DocumentWindow
	{
	public:
		MainWindow (juce::String name) : DocumentWindow (
					name, Colour(), DocumentWindow::allButtons)
		{
			LookAndFeel& def_look_and_feel =
					juce::Desktop::getInstance().getDefaultLookAndFeel();
			setBackgroundColour(def_look_and_feel.findColour(
										ResizableWindow::backgroundColourId));
			setUsingNativeTitleBar (true);
			setContentOwned (new MainComponent(), true);

   		#if JUCE_IOS || JUCE_ANDROID
			setFullScreen (true);
   		#else
			setResizable (true, true);
			setResizeLimits (300, 350, 10000, 10000);
			//centreWithSize (getWidth(), getHeight());
			centreWithSize (800, 500);
   		#endif

			setVisible (true);
		}

		void closeButtonPressed() override
		{
			// This is called when the user tries to close this window.
			// Here, we'll just ask the app to quit when this happens, but
			// you can change this to do  whatever you need.
			JUCEApplication::getInstance()->systemRequestedQuit();
		}

		/* Note: Be careful if you override any DocumentWindow methods - the
		 * base class uses a lot of them, so by overriding you might break its
		 * functionality. It's best to do all your work in your content
		 * component instead, but if you really have to override any
		 * DocumentWindow methods, make sure your subclass also calls
		 * the superclass's method.
		 */

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
	};

private:
	std::unique_ptr<MainWindow> mainWindow;
};



static void critical_error(char *file_str, int line_num, char *msg)
{
	String err_title("critical_error");
	String err_msg("");
	err_msg += "file: ";
	err_msg += file_str;
	err_msg += ": line(";
	err_msg += line_num;
	err_msg += ").\n";
	err_msg += msg;
	AlertWindow alert_win(err_title, err_msg, AlertWindow::NoIcon);
	alert_win.addButton(String("OK"), 0);
	alert_win.runModalLoop();
}
extern "C" {
	void c_critical_error(char *file_str, int line_num, char *msg)
	{
		critical_error(file_str, line_num, msg);
		exit(1);
	}
}


//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (NuAEC_RNNApplication)
