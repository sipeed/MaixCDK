// Copyright (c) 2022 Binbin Zhang (binbzha@qq.com)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#include <iostream>
#include <string>
#include <list>

#include "frontend/feature_pipeline.h"
#include "frontend/wav.h"
#include "kws/keyword_spotting.h"
#include "utils/log.h"
#include "maix_basic.hpp"
#include "maix_audio.hpp"
#include "maix_image.hpp"
#include "maix_vision.hpp"
#include "kaldi-native-fbank/csrc/online-feature.h"
#include "kaldi-native-fbank/csrc/feature-fbank.h"
#include "kaldi-native-fbank/csrc/feature-mfcc.h"
#include "kaldi-native-fbank/csrc/feature-window.h"
#include "kaldi-native-fbank/csrc/mel-computations.h"

using namespace maix;

class FeatureComputer
{
	knf::OnlineFbank *_fbank;
	knf::OnlineMfcc *_mfcc;
	bool _is_fbank;
public:
	knf::FbankOptions fbank_opts;
	knf::MfccOptions mfcc_opts;

	FeatureComputer(int num_bins, std::string feature_type)
	:fbank_opts(knf::FbankOptions()),mfcc_opts(knf::MfccOptions())
	{
		if (feature_type == "fbank") {
			fbank_opts.frame_opts.dither = 0;
			fbank_opts.frame_opts.samp_freq = 16000;
			fbank_opts.mel_opts.num_bins = num_bins;
			_fbank = new knf::OnlineFbank(fbank_opts);
			_is_fbank = true;
		} else {
			mfcc_opts.frame_opts.dither = 0;
			mfcc_opts.frame_opts.samp_freq = 16000;
			mfcc_opts.use_energy = false;
			mfcc_opts.energy_floor = 1.0;
			mfcc_opts.mel_opts.num_bins = num_bins;
			mfcc_opts.num_ceps = num_bins;
			_mfcc = new knf::OnlineMfcc(mfcc_opts);
			_is_fbank = false;
		}
	}
	~FeatureComputer() {
		if (_is_fbank) {
			if (_fbank) {
				delete _fbank;
				_fbank = nullptr;
			}
		} else {
			if (_mfcc) {
				delete _mfcc;
				_mfcc = nullptr;
			}
		}
	}

	void AcceptWaveform(const float *waveform, int32_t n) {
		if (_is_fbank) {
			_fbank->AcceptWaveform(fbank_opts.frame_opts.samp_freq, waveform, n);
		} else {
			_mfcc->AcceptWaveform(mfcc_opts.frame_opts.samp_freq, waveform, n);
		}
	}

	size_t NumFramesReady() {
		if (_is_fbank) {
			return _fbank->NumFramesReady();
		} else {
			return _mfcc->NumFramesReady();
		}
	}

	const float *GetFrame(size_t n) {
		if (_is_fbank) {
			return _fbank->GetFrame(n);
		} else {
			return _mfcc->GetFrame(n);
		}
	}

	void Pop(size_t n) {
		if (_is_fbank) {
			_fbank->Pop(n);
		} else {
			_mfcc->Pop(n);
		}
	}
};

const std::vector<std::string> classes10_name = {"unknown", "yes", "no", "up", "down", "left", "right", "on", "off", "stop", "go"};
const std::vector<std::string> classes1_name = {"hibela"};

std::vector<float> softmax(std::vector<float> input)
{
	float total = 0.;
	for(auto x : input)
	{
		total += std::exp(x);
	}

	std::vector<float> result;

	for(auto x : input)
	{
		result.push_back(std::exp(x) / total);
	}
	return result;
}

std::pair<std::string, float> find_max_prob(std::vector<float> input, std::vector<std::string> classes_name)
{
	if (input.size() == 1) {
		if (input[0] > 0.8) {
			return std::make_pair(classes_name[0], input[0]);
		} else {
			return std::make_pair("unknown", 0.);
		}
	} else {
		auto prob = softmax(input);
		float max_prob = 0.;
		int max_idx = -1;
		for (size_t i = 0; i < prob.size(); i ++) {
			if (prob[i] > max_prob) {
				max_prob = prob[i];
				max_idx = i;
			}
		}
		return std::make_pair(classes_name[max_idx], max_prob);
	}
}

static void wav_handle(int num_bins, int batch_size, std::string model_path, std::string feature_type, std::string wav_path) {
	wenet::WavReader wav_reader(wav_path);
	int num_samples = 0;
	num_samples = wav_reader.num_samples();

	FeatureComputer computer(num_bins, feature_type);
	wekws::KeywordSpotting spotter(model_path);

	computer.AcceptWaveform(wav_reader.data(), num_samples);

	auto ready_num = computer.NumFramesReady();
	for (size_t i = 0; i < ready_num - batch_size; i += batch_size) {
		std::vector<std::vector<float>> feats;
		for (size_t j = 0; j < (size_t)batch_size; ++ j) {
			auto frame = computer.GetFrame(i + j);

			feats.push_back(std::move(std::vector(frame, frame + num_bins)));
			computer.Pop(1);
		}

		std::vector<std::vector<float>> probs;
		spotter.Forward(feats, &probs);

		for (size_t m = 0; m < probs.size(); m ++) {
			auto prob = probs[m];
			if (prob.size() == 1) {
				auto result = find_max_prob(prob, classes1_name);
				log::info("classes name:%s, prob:%f", result.first.c_str(), result.second);
			} else {
				auto result = find_max_prob(prob, classes10_name);
				log::info("classes name:%s, prob:%f", result.first.c_str(), result.second);
			}
		}
	}
}

static std::vector<float> s16le_to_float(int16_t *data, size_t data_size)
{
	std::vector<float> data_f;
	data_f.resize(data_size);
	for (size_t i = 0; i < data_size; i ++) {
		// data_f[i] = (float)data[i] / 32768.0;
		data_f[i] = static_cast<float>(data[i]);
	}
	return data_f;
}


static void without_wav_handle(int num_bins, int batch_size, std::string model_path, std::string feature_type) {
	auto recorder = audio::Recorder("", 16000);
	recorder.volume(100);
	auto frame_size = recorder.frame_size();
	int num_samples = 512;

	FeatureComputer computer(num_bins, feature_type);log::info("model path:%s", model_path.c_str());
	wekws::KeywordSpotting spotter(model_path);

	auto disp = display::Display();
	auto img = image::Image(480, 320);

	size_t frame_offset = 0;
	std::vector<std::vector<float>> feats;
	std::list<std::pair<std::string, float>> max_prob_list;
	while (!app::need_exit()) {
		auto data = recorder.record(num_samples * frame_size);
		if (data) {
			auto data_s16 = (int16_t *)data->data;
			auto data_s16_size = data->data_len / 2;
			auto data_f = s16le_to_float(data_s16, data_s16_size);
			computer.AcceptWaveform(data_f.data(), data_f.size());
			free(data);
		}

		auto ready_frame_num = computer.NumFramesReady();//
		while (!app::need_exit()) {
			if (frame_offset >= ready_frame_num) {
				break;
			}

			if (feats.size() < (size_t)batch_size) {
				auto frame = computer.GetFrame(frame_offset);
				feats.push_back(std::move(std::vector(frame, frame + num_bins)));
				computer.Pop(1);
				frame_offset ++;
			}

			if (feats.size() >= (size_t)batch_size) {
				std::vector<std::vector<float>> probs;
				spotter.Forward(feats, &probs);

				std::pair<std::string, float> max_prob = {"", 0};
				for (size_t m = 0; m < probs.size(); m ++) {
					auto prob = probs[m];
					if (prob.size() == 1) {
						auto result = find_max_prob(prob, classes1_name);
						if (result.second > 0.005) {
							log::info("classes name:%s, prob:%f", result.first.c_str(), result.second);
						}

						if (result.second > max_prob.second) {
							max_prob = result;
						}
					} else {
						auto result = find_max_prob(prob, classes10_name);
						if (result.second > 0.005) {
							log::info("classes name:%s, prob:%f", result.first.c_str(), result.second);
						}

						if (result.second > max_prob.second) {
							max_prob = result;
						}
					}
					max_prob_list.push_back(max_prob);
				}
				feats.clear();

				if (max_prob_list.size() > 5) {
					max_prob_list.pop_front();
				}

				img.clear();
				img.draw_string(40, 40, "Say left/up/right/down", image::COLOR_WHITE, 1.5);
				int offset = 0;
				for (auto it = max_prob_list.begin(); it != max_prob_list.end(); it ++) {
					image::Color color = image::COLOR_WHITE;
					if (it->first != "unknown") {
						if (it->second > 0.8) {
							color = image::COLOR_GREEN;
						} else if (it->second > 0.5) {
							color = image::COLOR_YELLOW;
						}
					}
					img.draw_string(40, 80 + offset * 40, it->first + ", " + std::to_string(it->second), color, 1.5);
					offset ++;
				}
				disp.show(img);
			}
		}
	}
}

int main(int argc, char* argv[]) {
	int num_bins = 80;
	int batch_size = 40;
	std::string model_path = "/root/models/kws_speechcommand.onnx";
	std::string feature_type = "mfcc";
	std::string wav_path;
	bool use_wav = false;
	if (argc > 1) {
		if (!strcmp(argv[1], "-h")) {
			log::info("Usage: kws_main fbank_dim(int) batch_size(int) "
			"kws_model_path feature_type(fbank,mfcc) test_wav_path");
			exit(0);
		} else {
			num_bins = std::stoi(argv[1]);
		}
	}

	if (argc > 2) {
		batch_size = std::stoi(argv[2]);
	}

	if (argc > 3) {
		model_path = argv[3];
	}

	if (argc > 4) {
		feature_type = argv[4];
	}

	if (argc > 5) {
		wav_path = argv[5];
		use_wav = true;
	}

	log::info("Input parameters are:");
	log::info("num_bins:%d batch_size:%d model_path:%s wav_path:%s", num_bins, batch_size, model_path.c_str(), wav_path.c_str());
	if (use_wav) {
		wav_handle(num_bins, batch_size, model_path, feature_type, wav_path);
	} else {
		without_wav_handle(num_bins, batch_size, model_path, feature_type);
	}

	return 0;
}
