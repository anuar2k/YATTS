#pragma once
#include "../pch.h"

#include <functional>

class FloatConverters {
	public:
	enum class Converter {
		NONE,
		MS_TO_KMH,
		MS_TO_MPH,
		PSI_TO_MPA,
		PSI_TO_BAR,
		C_TO_F,
		C_TO_K,
		L_TO_GAL,
		KM_TO_MI,
		L100KM_TO_MPG
	};

	static const std::function<double(double)>& get_converter(Converter conv) {
		switch (conv) {
			case Converter::MS_TO_KMH:
				return [](double val) { return val * 3.6; };
			case Converter::MS_TO_MPH:
				return [](double val) { return val * 2.2369362920544; };
			case Converter::PSI_TO_MPA:
				return [](double val) { return val * 0.006894759086775369; };
			case Converter::PSI_TO_BAR:
				return [](double val) { return val * 0.0689475729; };
			case Converter::C_TO_F:
				return [](double val) { return (val * 9 / 5) + 32; };
			case Converter::C_TO_K:
				return [](double val) { return val + 273.15; };
			case Converter::L_TO_GAL:
				return [](double val) { return val * 0.264172052; };
			case Converter::KM_TO_MI:
				return [](double val) { return val * 0.621371192; };
			case Converter::L100KM_TO_MPG:
				return [](double val) { return 235.214583 / val; };
		}
		
		return [](double val) { return val; };
	}

	private:
	FloatConverters() {

	}
};