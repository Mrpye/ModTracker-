#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <limits>
#include <stdexcept>
#include "stringhelper.h"
#include "tangible_filesystem.h"

//C# TO C++ CONVERTER NOTE: Forward class declarations:


	class MusicModLib
	{
//		#region Fields
	private:
		TuningLookup *tuning = new TuningLookup();
	public:
		int CHANNEL_COUNT = 1;
	private:
		std::vector<Channel*> channel;
	public:
		Song *song = nullptr;
	private:
		long long last_time = 0;
		long long tempo_time_elapse = 0;
	public:
		int TEMPO = 120;
	private:
		std::vector<std::vector<float>> sound_bank;
//		#endregion


	public:
		virtual ~MusicModLib()
		{
			delete tuning;
			delete song;
		}

		void LoadSong(const std::wstring &filename);

		long long GetMillisecond();


		double Update();

		std::vector<float> LoadSample(const std::wstring &filename);


	};

	class TuningLookup
	{
	private:
		std::unordered_map<std::wstring, double> privateNote_Lookup = std::unordered_map<std::wstring, double>();

	public:
		std::unordered_map<std::wstring, double> getNote_Lookup() const;
		void setNote_Lookup(const std::unordered_map<std::wstring, double> &value);
		TuningLookup();
		double GetFeq(const std::wstring &note);
	};

	class SongNote
	{
	public:
		std::wstring note;
		double feq = 0;
		double volume = 0;
		int sound = 0;

		SongNote(const std::wstring &note, double feq, double volume);

		SongNote() = default;
	};

	class SongNoteGroup
	{
	public:
		std::vector<SongNote> track = {SongNote(), SongNote(), SongNote(), SongNote(), SongNote(), SongNote(), SongNote(), SongNote()};


	};

	class SongBlock
	{
	public:
		std::vector<SongNoteGroup*> track_lines = std::vector<SongNoteGroup*>();
	private:
		SongNoteGroup *track_line = nullptr;

	public:
		virtual ~SongBlock()
		{
			delete track_line;
		}

		SongBlock();

		int current_track_line = -1;

		bool Next_Track_Line();

		SongNoteGroup *Get_Current_TrackLine();

		void ReadSongBlock(System::IO::StreamReader *file);
	};

	class Arrangment
	{
	public:
		std::vector<SongNoteGroup*> track_line = std::vector<SongNoteGroup*>();

		Arrangment();
	};

	/// <summary>
	/// The song hold all the elements together
	/// </summary>
	class Song
	{
	private:
		std::vector<std::wstring> privatesample_list = std::vector<std::wstring>();

	public:
		std::vector<SongBlock*> song_blocks = std::vector<SongBlock*>();
	private:
		SongBlock *current_song_block = nullptr;
	public:
		virtual ~Song()
		{
			delete current_song_block;
		}

		std::vector<std::wstring> getsample_list() const;
		void setsample_list(const std::vector<std::wstring> &value);
		int tempo = 0;
		int channels = 0;
		Song();

		SongBlock *Get_Current_Song_Block();

		SongNoteGroup *Get_Current_TrackLine();

		void Next_Step();

		void LoadSong(const std::wstring &file_name);

		void Read_Sample_Block(StreamReader *file);
	};


	class Channel
	{
	private:
		double current_wav_pos = 0;
	public:
		std::vector<std::vector<float>> sound_data;
	private:
		double volume = 0;
		double feq = 0;
	public:
		bool is_playing = false;
		int last_index = -1;
		double last_feq = 0;
		int current_sound = 0;
		void Set_Sound_Data(std::vector<std::vector<float>> &sound_data);

		void PlayNote(double feq, double volume, int sound);

		void StopNote();


		double Update();




	};






	void MusicModLib::LoadSong(const std::wstring &filename)
	{
		this->song = new Song();

		if (!FileSystem::fileExists(filename))
		{
			throw std::runtime_error("Missing file");
		}

		this->song = new Song();
		this->song->LoadSong(filename);

		CHANNEL_COUNT = this->song->channels;
		this->TEMPO = this->song->tempo;

		this->channel = std::vector<Channel*>(CHANNEL_COUNT);
		//SET UP THE CHANNELS
		for (int i = 0; i < CHANNEL_COUNT; i++)
		{
			this->channel[i] = new Channel();
		}


		//now we load in the sample data
		this->sound_bank = std::vector<std::vector<float>>(this->song->getsample_list().size()());

		std::wstring data_path = FileSystem::getDirectoryName(filename);
		std::wstring song_name = System::IO::Path::GetFileNameWithoutExtension(filename);
		std::wstring sample_path = FileSystem::combine(data_path, song_name);
		for (int i = 0; i < this->song->getsample_list().size()(); i++)
		{
			std::wstring full_sample_path = FileSystem::combine(sample_path, this->song->getsample_list()[i]);
			if (!FileSystem::fileExists(filename))
			{
				throw std::runtime_error("Missing file");
			}
			this->sound_bank[i] = this->(LoadSamplefull_sample_path);
		}

		for (int i = 0; i < CHANNEL_COUNT; i++)
		{
			this->channel[i]->Set_Sound_Data(this->sound_bank);
		}

		tempo_time_elapse = (30000 / this->TEMPO);

		last_time = this->GetMillisecond();
	}

	long long MusicModLib::GetMillisecond()
	{
		long long milliseconds = DateTime::Now.Ticks / TimeSpan::TicksPerMillisecond;
		return milliseconds;
	}

	double MusicModLib::Update()
	{
		double val = 0;
		//This check to see if we need to progress the song along
		long long time = this->GetMillisecond() - this->last_time;
		if (time > this->tempo_time_elapse)
		{
			this->song->Next_Step();
			//Let reset the tempo timer
			this->last_time = this->GetMillisecond();
		}

		for (int i = 0; i < channel.size(); i++)
		{
			SongNote sn = this->song->Get_Current_TrackLine()->track[i];
			if (this->song->Get_Current_Song_Block()->current_track_line != this->channel[i]->last_index)
			{
				this->channel[i]->last_index = this->song->Get_Current_Song_Block()->current_track_line;
				// sn.feq = 300/(this.channel[i].ldata.Length*0.1);
				if (sn.feq == -1)
				{
					sn.feq = this->channel[i]->last_feq;
				}
				else
				{
					this->channel[i]->StopNote();
					if (sn.note != L"")
					{
						this->channel[i]->last_feq = sn.feq;
						this->channel[i]->PlayNote(sn.feq, sn.volume,sn.sound);
					}
				}
			}
			val += this->channel[i]->Update();
		}


		return static_cast<float>(val);
	}

	std::vector<float> MusicModLib::LoadSample(const std::wstring &filename)
	{
		 std::vector<float> sound_data;

		try
		{
//C# TO C++ CONVERTER NOTE: The following 'using' block is replaced by its C++ equivalent:
//ORIGINAL LINE: using (FileStream fs = File.Open(filename, FileMode.Open))
			{
				FileStream fs = File::Open(filename, FileMode::Open);
				BinaryReader *reader = new BinaryReader(fs);

				// chunk 0
				int chunkID = reader->ReadInt32();
				int fileSize = reader->ReadInt32();
				int riffType = reader->ReadInt32();

				// chunk 1
				int fmtID = reader->ReadInt32();
				int fmtSize = reader->ReadInt32(); // bytes for this chunk
				int fmtCode = reader->ReadInt16();
				int channels = reader->ReadInt16();
				int sampleRate = reader->ReadInt32();
				int byteRate = reader->ReadInt32();
				int fmtBlockAlign = reader->ReadInt16();
				int bitDepth = reader->ReadInt16();

				if (fmtSize == 18)
				{
					// Read any extra values
					int fmtExtraSize = reader->ReadInt16();
					reader->ReadBytes(fmtExtraSize);
				}

				// chunk 2
				int dataID = reader->ReadInt32();
				int bytes = reader->ReadInt32();

				// DATA!
				std::vector<unsigned char> byteArray = reader->ReadBytes(bytes);

				int bytesForSamp = bitDepth / 8;
				int samps = bytes / bytesForSamp;

				std::vector<float> asFloat;
				switch (bitDepth)
				{
					case 64:
					{
						std::vector<double> asDouble(samps);
						Buffer::BlockCopy(byteArray, 0, asDouble, 0, bytes);
						asFloat = Array::ConvertAll(asDouble, [&] (std::any e)
						{
							(float)e;
						});
						break;

					}
					case 32:
						asFloat = std::vector<float>(samps);
						Buffer::BlockCopy(byteArray, 0, asFloat, 0, bytes);
						break;

					case 24:
						asFloat = std::vector<float>(samps);
						Buffer::BlockCopy(byteArray, 0, asFloat, 0, bytes);
						break;

					case 16:
					{
						std::vector<short> asInt16(samps);
						Buffer::BlockCopy(byteArray, 0, asInt16, 0, bytes);
						asFloat = Array::ConvertAll(asInt16, [&] (std::any e)
						{
						delete reader;
							return e / static_cast<float>(std::numeric_limits<short>::max());
						});
						break;



					}
				}

				switch (channels)
				{
					case 1:
						sound_data = asFloat;
						//R = null;

						delete reader;
						return sound_data;

					case 2:
						sound_data = std::vector<float>(samps);
						// R = new float[samps];
						for (int i = 0, s = 0; i < samps; i++)
						{
							sound_data[i] = asFloat[s++];
							//   R[i] = asFloat[s++];
						}

					   delete reader;
					   return sound_data;
					default:
						return nullptr;
				}

				delete reader;
			}
		}
		catch (...)
		{
			Debug::WriteLine(L"...Failed to load note: " + filename);
			return nullptr;
			//left = new float[ 1 ]{ 0f };
		}

	}

	std::unordered_map<std::wstring, double> TuningLookup::getNote_Lookup() const
	{
		return privateNote_Lookup;
	}

	void TuningLookup::setNote_Lookup(const std::unordered_map<std::wstring, double> &value)
	{
		privateNote_Lookup = value;
	}

	TuningLookup::TuningLookup()
	{
		double step = 0.5 / 16;
		getNote_Lookup().emplace(L"C", 0.5);
		getNote_Lookup().emplace(L"C#", 0.5 + (step * 1));
		getNote_Lookup().emplace(L"Db", 0.5 + (step * 2));
		getNote_Lookup().emplace(L"D", 0.51 + (step * 3));
		getNote_Lookup().emplace(L"D#", 0.5 + (step * 4));
		getNote_Lookup().emplace(L"Eb", 0.5 + (step * 5));
		getNote_Lookup().emplace(L"E", 0.5 + (step * 6));
		getNote_Lookup().emplace(L"F", 0.5 + (step * 7));
		getNote_Lookup().emplace(L"F#", 0.51 + (step * 8));
		getNote_Lookup().emplace(L"Gb", 0.51 + (step * 9));
		getNote_Lookup().emplace(L"G", 0.5 + (step * 10));
		getNote_Lookup().emplace(L"G#", 0.5 + (step * 11));
		getNote_Lookup().emplace(L"Ab", 0.5 + (step * 12));
		getNote_Lookup().emplace(L"A", 0.5 + (step * 13));
		getNote_Lookup().emplace(L"A#", 0.5 + (step * 14));
		getNote_Lookup().emplace(L"Bb", 0.5 + (step * 15));
		getNote_Lookup().emplace(L"B", 0.5 + (step * 16));


	}

	double TuningLookup::GetFeq(const std::wstring &note)
	{
		if (StringHelper::trim(note)->length() == 2)
		{
			double feq = getNote_Lookup()[note.substr(0, 1)];
			int oct = std::stoi(note.substr(1, 1));
			double res = feq + oct; // Math.Pow(feq, (oct + 1));
			return res;
		}
		else if (StringHelper::trim(note)->length() == 3)
		{
			double feq = getNote_Lookup()[note.substr(0, 2)];
			int oct = std::stoi(note.substr(2, 1));
			double res = feq + oct;
			return res;
		}
		else
		{
			if (note == L"-")
			{
				return -1;
			}
			else
			{
				return 0;
			}

		}
	}

	SongNote::SongNote(const std::wstring &note, double feq, double volume)
	{
		this->note = note;
		this->feq = 0;
		this->volume = 0;
		this->sound = 6;
	}

	SongBlock::SongBlock()
	{
	}

	bool SongBlock::Next_Track_Line()
	{
		current_track_line += 1;
		if (this->current_track_line > track_lines.size() - 1)
		{
			this->current_track_line = 0;
			this->track_line = this->track_lines[this->current_track_line];
			return true;
		}
		else
		{
			this->track_line = this->track_lines[this->current_track_line];
		}
		return false;
	}

	SongNoteGroup *SongBlock::Get_Current_TrackLine()
	{
		if (this->current_track_line == -1)
		{
			this->current_track_line = 0;
		}
		if (this->track_line == nullptr)
		{
			this->track_line = this->track_lines[this->current_track_line];
		}
		return this->track_line;
	}

	void SongBlock::ReadSongBlock(System::IO::StreamReader *file)
	{
		std::wstring line;
		TuningLookup *tuning = new TuningLookup();
		while ((line = file->ReadLine()) != L"")
		{
			if (StringHelper::toUpper(line) == L"BLOCK_END")
			{
				delete tuning;
				return;
			}

			//READ THE DATA
			//Split the track data
			SongNoteGroup *new_song_group = new SongNoteGroup();
			std::vector<std::wstring> TrackData = StringHelper::split(line, L',');
			for (int i = 0; i < TrackData.size(); i++)
			{
				//we need to split the data and store in out song note
				std::vector<std::wstring> note_data = StringHelper::split(TrackData[i], L':');
				new_song_group->track[i].note = StringHelper::trim(note_data[0]);
				new_song_group->track[i].feq = tuning->GetFeq(StringHelper::trim(note_data[0]));
				new_song_group->track[i].volume = std::stod(StringHelper::trim(note_data[1]));
				new_song_group->track[i].sound = std::stoi(StringHelper::trim(note_data[2]));

			}
			this->track_lines.push_back(new_song_group);

//C# TO C++ CONVERTER TODO TASK: A 'delete new_song_group' statement was not added since new_song_group was passed to a method or constructor. Handle memory management manually.
		}

		tuning = nullptr;

		delete tuning;
	}

	Arrangment::Arrangment()
	{
	}

	std::vector<std::wstring> Song::getsample_list() const
	{
		return privatesample_list;
	}

	void Song::setsample_list(const std::vector<std::wstring> &value)
	{
		privatesample_list = value;
	}

	Song::Song()
	{
	}

	SongBlock *Song::Get_Current_Song_Block()
	{
		return this->current_song_block;
	}

	SongNoteGroup *Song::Get_Current_TrackLine()
	{
		if (current_song_block == nullptr)
		{
			current_song_block = song_blocks[0];
		}
		return this->current_song_block->Get_Current_TrackLine();
	}

	void Song::Next_Step()
	{
		current_song_block = song_blocks[0];
		current_song_block->Next_Track_Line();
	}

	void Song::LoadSong(const std::wstring &file_name)
	{
		int counter = 0;
		std::wstring line;

		System::IO::StreamReader *file = new System::IO::StreamReader(file_name);
		while ((line = file->ReadLine()) != L"")
		{
			if (StringHelper::toUpper(line) == L"BLOCK_START")
			{
				SongBlock *song_block = new SongBlock();
				song_block->ReadSongBlock(file);
				this->song_blocks.push_back(song_block);

//C# TO C++ CONVERTER TODO TASK: A 'delete song_block' statement was not added since song_block was passed to a method or constructor. Handle memory management manually.
			}
			else if (StringHelper::toUpper(line) == L"BLOCK_SAMPES")
			{
				Read_Sample_Block(file);
			}
			else if (StringHelper::startsWith(StringHelper::toUpper(line), L"TEMPO"))
			{
				this->tempo = std::stoi(StringHelper::split(line, L' ')[1]);
			}
			else if (StringHelper::startsWith(StringHelper::toUpper(line), L"CHANNELS"))
			{
				this->channels = std::stoi(StringHelper::split(line, L' ')[1]);
			}
			counter++;
		}

		file->Close();

//C# TO C++ CONVERTER TODO TASK: A 'delete file' statement was not added since file was passed to a method or constructor. Handle memory management manually.
	}

	void Song::Read_Sample_Block(System::IO::StreamReader *file)
	{
		std::wstring line;
		TuningLookup *tuning = new TuningLookup();
		while ((line = file->ReadLine()) != L"")
		{
			if (StringHelper::toUpper(line) == L"BLOCK_END")
			{
				delete tuning;
				return;
			}
			this->getsample_list().push_back(StringHelper::trim(line));
		}

		delete tuning;
	}

	void Channel::Set_Sound_Data(std::vector<std::vector<float>> &sound_data)
	{
		this->sound_data = sound_data;
	}

	void Channel::PlayNote(double feq, double volume, int sound)
	{
		if (is_playing == true)
		{
			return;
		}
		is_playing = true;
		this->volume = volume;
		this->feq = feq;
		this->current_sound = sound;
		this->current_wav_pos = 0; //-= feq;

	}

	void Channel::StopNote()
	{
		this->current_wav_pos = 0;
		this->is_playing = false;
	}

	double Channel::Update()
	{
		float val = 0;
		if (is_playing == true)
		{
			this->current_wav_pos += this->feq;
			if (this->current_wav_pos > this->sound_data[this->current_sound].size() - 1)
			{
				this->current_wav_pos = 0;
				is_playing = false;
			}
			val = this->sound_data[this->current_sound][static_cast<int>(this->current_wav_pos)] * static_cast<float>(this->volume);
		}
		return val;
	}
