#pragma once

namespace Game {
	namespace VSRG {
		class ScoreKeeper;
		struct Difficulty;
		class TrackNote;
		class Song;

		enum TimingType
		{
			TT_TIME,
			TT_BEATS,
			TT_PIXELS
		};

		class Mechanics
		{
		public:
			typedef std::function<void(double, uint32_t, bool, bool)> HitEvent;
			typedef std::function<void(double, uint32_t, bool, bool, bool)> MissEvent;
			typedef std::function<void(uint32_t)> KeysoundEvent;

		protected:

			Difficulty *CurrentDifficulty;
			std::shared_ptr<ScoreKeeper> score_keeper;

		public:

			virtual ~Mechanics() = default;

			// These HAVE to be set before anything else is called.
			std::function <bool(uint32_t)> IsLaneKeyDown;
			std::function <void(uint32_t, bool)> SetLaneHoldingState;
			KeysoundEvent PlayNoteSoundEvent;
			HitEvent HitNotify;
			MissEvent MissNotify;

			virtual void Setup(VSRG::Difficulty *Difficulty, std::shared_ptr<ScoreKeeper> scoreKeeper);

			// If returns true, don't judge any more notes.
			virtual bool OnUpdate(double SongTime, VSRG::TrackNote* Note, uint32_t Lane) = 0;

			// If returns true, don't judge any more notes.
			virtual bool OnPressLane(double SongTime, VSRG::TrackNote* Note, uint32_t Lane) = 0;

			// If returns true, don't judge any more notes either.
			virtual bool OnReleaseLane(double SongTime, VSRG::TrackNote* Note, uint32_t Lane) = 0;

			virtual TimingType GetTimingKind() = 0;
		};

		class RaindropMechanics : public Mechanics
		{
			bool forcedRelease;
		public:
			RaindropMechanics(bool forcedRelease);
			bool OnUpdate(double SongTime, VSRG::TrackNote* Note, uint32_t Lane) override;
			bool OnPressLane(double SongTime, VSRG::TrackNote* Note, uint32_t Lane) override;
			bool OnReleaseLane(double SongTime, VSRG::TrackNote* Note, uint32_t Lane) override;

			TimingType GetTimingKind() override;
		};

		class O2JamMechanics : public Mechanics
		{
		public:

			bool OnUpdate(double SongBeat, VSRG::TrackNote* Note, uint32_t Lane) override;
			bool OnPressLane(double SongBeat, VSRG::TrackNote* Note, uint32_t Lane) override;
			bool OnReleaseLane(double SongBeat, VSRG::TrackNote* Note, uint32_t Lane) override;

			TimingType GetTimingKind() override;
		};
	}
}