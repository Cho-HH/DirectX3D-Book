#pragma once

class Timer
{
public:
	Timer();
	~Timer() = default;

public:
	float		TotalTime() const;
	float		DeltaTime() const;
	void		Start();
	void		Resume();
	void		Stop();
	void		Update();
	inline void		SetScale(float scale);

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

void Timer::SetScale(float scale)
{
	mfScale = scale;
}

