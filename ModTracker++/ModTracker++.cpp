// ModTracker++.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "olcConsoleGameEngine.h"
#include "MusicModLib.h"
using namespace std;


class ModTracker : public olcConsoleGameEngine
{
public:
	ModTracker()
	{
		m_sAppName = L"Video";
	}

private:
	  MusicModLib *mm = nullptr;

protected:
	virtual bool OnUserCreate()
	{
	
		//*******************************
		//Let read in the song data first
		//*******************************
		this->mm = new MusicModLib();
		this->mm->LoadSong(LR"(D:\Projects\Net\MusicTracker\Music\song.txt)");


		this->EnableSound();
		return true;
	}

	virtual float onUserSoundSample(int nChannel, float fGlobalTime, float fTimeStep) {
		return 0;
	}

	virtual bool OnUserUpdate(float fElapsedTime)
	{
		
		return true;
	}

};

int main()
{
	ModTracker game;
	game.ConstructConsole(320, 240, 4, 4);
	game.Start();
	return 0;
}