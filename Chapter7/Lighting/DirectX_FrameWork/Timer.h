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

	__int64	mllBaseTime;
	__int64 mllPausedTime;
	__int64 mllStopTime;
	__int64 mllPrevTime;
	__int64 mllCurrTime;

	bool		mbStopped;
};

void Timer::SetScale(float scale)
{
	mfScale = scale;
}

