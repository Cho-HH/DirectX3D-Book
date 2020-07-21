#pragma once
class Timer
{
public:
	Timer();

public:
	float		TotalTime() const;
	float		DeltaTime() const;
	void		Start();
	void		Resume();
	void		Stop();
	void		Update();
	void		SetScale(float scale);

private:
	double		mdSecondsPerCount;
	double		mdDeltaTime;
	float		mfScale;

	long long	mllBaseTime;
	long long	mllPausedTime;
	long long	mllStopTime;
	long long	mllPrevTime;
	long long	mllCurrTime;

	bool		mbStopped;
};
