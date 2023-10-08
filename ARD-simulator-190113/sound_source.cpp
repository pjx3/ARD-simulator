#include "sound_source.h"
#include "gaussian_source.h"
#include "simulation.h"
#include "partition.h"
#include <iostream>


SoundSource::SoundSource(int x, int y, int z) :x_(x), y_(y), z_(z)
{
	static int id_generator = 0;
	id_ = id_generator++;
	std::string filename;
	std::string dir_name = std::to_string(Simulation::m_dh) + "_" + std::to_string(Partition::m_absorption);
	filename = "./output/" + dir_name + "/source_" + std::to_string(id_) + ".txt";
	source_.open(filename, std::ios::out);

}


SoundSource::~SoundSource()
{
}

std::vector<std::shared_ptr<SoundSource>> SoundSource::ImportSources(std::string path)
{
	std::vector<std::shared_ptr<SoundSource>> sources;

	std::ifstream file;
	file.open(path, std::ifstream::in);
	while (file.good())
	{
		int x, y, z;
		file >> x >> y >> z;
		if (file.eof()) break;

		float const xh = x / Simulation::m_dh;
		float const yh = y / Simulation::m_dh;
		float const zh = z / Simulation::m_dh;

		sources.push_back(std::make_shared<GaussianSource>((int)xh, (int)yh, (int)zh));
	}
	file.close();
	for (auto source : sources)
	{
		source->RecordSource();
	}
	return sources;
}

void SoundSource::RecordSource()
{
	for (int t = 0; t < Simulation::m_duration / Simulation::m_dt; t++)
	{
		source_ << this->SampleValue((real_t)t) << std::endl;
	}
	source_.close();
}
