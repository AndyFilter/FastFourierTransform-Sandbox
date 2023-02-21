#include <stdio.h>
#include <stdlib.h>
//#include <chrono>

#include "External/ImGui/imgui_internal.h"
#include "External/ImGui/imgui.h"
#include "Utils.hpp"
#include "GUI/gui.h"

static std::complex<SCALAR>* wave;
static std::complex<SCALAR>* wave2;
static std::complex<SCALAR>* wave3;
static std::complex<SCALAR>* waveFFT;
static std::complex<SCALAR>* waveFFT_Cooley;
static std::complex<SCALAR>* waveFFT_Cooley_Real;
static float frequencies[3]{ 85, 0, 0 };
//static int frequencies[3]{ 86, 0, 0 };
static float shifts[3]{ 0, 0, 0 };

void UpdateWaves() {
	GenerateWaveClean(wave, DATA_SIZE, frequencies[0], shifts[0]);
	GenerateWaveClean(wave2, DATA_SIZE, frequencies[1], shifts[1]);
	GenerateWaveClean(wave3, DATA_SIZE, frequencies[2], shifts[2]);

	for (int i = 0; i < DATA_SIZE; i++) {
		wave[i] += wave2[i];
		wave[i] += wave3[i];
	}

	// Setup arrays for Cooley FFT
	//memset(waveFFT_Cooley_Real, 0, DATA_SIZE / 2);
	//memcpy(waveFFT_Cooley, wave, DATA_SIZE * sizeof(std::complex<SCALAR>));
	//memcpy(waveFFT_Cooley + DATA_SIZE, wave, DATA_SIZE * sizeof(std::complex<SCALAR>));
	//auto fftStart = std::chrono::system_clock::now();
	fft(wave, waveFFT, POWER_2_DATA_SIZE);
	//auto fftDuration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - fftStart).count();
	//
	//auto cooleyFftStart = std::chrono::system_clock::now();
	//fft_Cooley(waveFFT_Cooley, DATA_SIZE);
	//auto cooleyDuration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - cooleyFftStart).count();
	//printf("FFT took: %lld and Cooley took: %lld\n", fftDuration, cooleyDuration);
	//
	// Input values are all real so we can "ignore" all the values above (DATA_SIZE/2) and multiply all below by 2
	//std::copy(waveFFT_Cooley, waveFFT_Cooley + DATA_SIZE/2, waveFFT_Cooley_Real);
	for (int i = 0; i < DATA_SIZE / 2; i++) {
		//waveFFT_Cooley_Real[i] *= 2;
		waveFFT[i] *= 2;
	}

}

int OnGui()
{
	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();

	ImVec2 regAvail = ImGui::GetContentRegionAvail();
	regAvail.y -= style.ItemSpacing.y + ImGui::GetFrameHeightWithSpacing() * 2;

	// ------ PHASE SHIFT -------
	// This was used for debugging purposes
	//for (int i = 0; i < 3; ++i) {
	//	if (frequencies[i] <= 0)
	//		continue;
	// 
	//	int binF = (int)floor(frequencies[i] * ((float)(SAMPLE_RATE)));
	//	int bin = (int)roundf(frequencies[i] * ((float)(SAMPLE_RATE)));
	//	int binC = (int)ceil(frequencies[i] * ((float)(SAMPLE_RATE)));
	//	ImGui::Text("(%f): Shift: %lf", ((float)binF * SAMPLE_RATE / (float)DATA_SIZE), std::arg(waveFFT_Cooley_Real[binF]));
	//	ImGui::Text("(%f): Shift: %lf", ((float)bin * SAMPLE_RATE / (float)DATA_SIZE), std::arg(waveFFT_Cooley_Real[bin]));
	//	ImGui::Text("(%f): Shift: %lf", ((float)binC * SAMPLE_RATE / (float)DATA_SIZE), std::arg(waveFFT_Cooley_Real[binC]));
	//}

	ImGui::BeginGroup();
	// We display only first (DATA_SIZE/5) values so the wave is more readable
	ImGui::PlotLines("Wave 1", [](void* data, int idx) { return (float)wave[idx].real(); }, nullptr, DATA_SIZE/5, 0, "Sine Waves Product", FLT_MAX, FLT_MAX, { regAvail.x, regAvail.y / 2.f });

	ImGui::BeginGroup();

	ImGui::PushItemWidth(regAvail.x - 400);

	if (ImGui::DragFloat3("Frequencies", frequencies, 1, 0, SAMPLE_RATE*(DATA_SIZE/2 - 1), "%.2fHz"))
		UpdateWaves();

	if (ImGui::SliderFloat3("Phase Shifts", shifts, -PI/2+0.001, PI/2-0.001))
		UpdateWaves();

	ImGui::PopItemWidth();

	ImGui::EndGroup();

	ImGui::SameLine(); ImGui::Dummy({ 4, 10 }); ImGui::SameLine();
	ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical); ImGui::SameLine();
	ImGui::SameLine(); ImGui::Dummy({ 4, 10 }); ImGui::SameLine();

	static bool showImaginaryPart = false, showRealPart = false, showAmplitude = true, showPhaseShift = false;

	ImGui::BeginGroup();
	ImGui::Checkbox("Real Part", &showRealPart);
	ImGui::SameLine();
	ImGui::Checkbox("Imaginary Part", &showImaginaryPart);
	//ImGui::SameLine();
	ImGui::Checkbox("Phase Shift", &showPhaseShift);
	ImGui::SameLine();
	ImGui::Checkbox("Amplitude", &showAmplitude);
	ImGui::EndGroup();

	auto pos = ImGui::GetCursorPos();
	//ImGui::PlotLines("##FFT", [](void* data, int idx) { return showAmplitude ? (float)std::abs(waveFFT[idx]) : 0; }, nullptr, DATA_SIZE, 0, "FFT", FLT_MAX, FLT_MAX, {regAvail.x, regAvail.y / 2.f});

	// First render a single frame, and then render all the plots on top of each other without the frames
	ImGui::RenderFrame(pos, { regAvail.x + pos.x, regAvail.y / 2.f + pos.y }, ImColor(style.Colors[ImGuiCol_FrameBg]), true, style.FrameRounding);

	ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0,0,0,0 });

	if (showAmplitude) {
		ImGui::PushStyleColor(ImGuiCol_PlotLines, { 0.9f, 0.9f, 0.9f,0.9f });
		ImGui::PushStyleColor(ImGuiCol_Text, { 0.9f, 0.9f, 0.9f,0.9f });
		ImGui::SetCursorPos(pos);
		ImGui::PlotLines("##FFTCooley", [](void* data, int idx) { return showAmplitude ? (float)std::abs(waveFFT[idx]) : 0; }, nullptr, DATA_SIZE / 2, 0, "FFT", FLT_MAX, FLT_MAX, { regAvail.x, regAvail.y / 2.f });
	}

	if (showPhaseShift) {
		ImGui::PushStyleColor(ImGuiCol_PlotLines, { 0.2f, 0.32f, 0.9f, 0.4f });
		ImGui::PushStyleColor(ImGuiCol_Text, { 0.4f, 0.52f, 0.9f, 0.6f });
		ImGui::SetCursorPos(pos);
		ImGui::PlotLines("##PhaseShiftPlot",
			[](void* data, int idx) { 
				// Filter out only the significant values.
				// Each "Bin" in FFT's result may have a "amplitude" ranging from 0 to DATA_SIZE.
				// So we take half of that and get only the bins where the values is bigger than that.
				return (std::abs(waveFFT[idx]) > (DATA_SIZE / 2.f)) ? (float)std::arg(waveFFT[idx]) : 0;
			},
			nullptr, DATA_SIZE / 2, 0, "", -PI, PI, { regAvail.x, regAvail.y / 2.f }
			);
		ImGui::RenderTextClipped(pos, { regAvail.x + pos.x, pos.y + ImGui::GetTextLineHeight() + (showAmplitude ? (style.ItemSpacing.y*2 + style.FramePadding.y*3) : style.FramePadding.y) }, "Phase Shift", NULL, NULL, {0.5, 1.f});
		//ImGui::RenderTextClipped(ImVec2(10, 200), ImVec2(100, 500), "texasdasdat", NULL, NULL, style.ButtonTextAlign);
		//ImGui::PlotLines("##Im(FFT)", [](void* data, int idx) { return std::abs(waveFFT[idx]) > 250.f ? (float)atan2(waveFFT[idx].imag(), waveFFT[idx].real()) : 0; }, nullptr, DATA_SIZE, 0, "Im(FFT)               ", FLT_MAX, FLT_MAX, { regAvail.x, regAvail.y / 2.f });
	}

	if (showImaginaryPart) {
		ImGui::PushStyleColor(ImGuiCol_PlotLines, { 0.2f, 0.82f, 0.3f, 0.4f });
		ImGui::PushStyleColor(ImGuiCol_Text, { 0.2f, 0.82f, 0.3f,0.5f });
		ImGui::SetCursorPos(pos);
		ImGui::PlotLines("##Im(FFT)", [](void* data, int idx) { return (float)waveFFT[idx].imag(); }, nullptr, DATA_SIZE / 2, 0, "Im(FFT)\t\t\t\t\t ", -50, 50, { regAvail.x, regAvail.y / 2.f });

		//ImGui::PlotLines("##Im(FFT)", [](void* data, int idx) { return std::abs(waveFFT[idx]) > 250.f ? (float)atan2(waveFFT[idx].imag(), waveFFT[idx].real()) : 0; }, nullptr, DATA_SIZE, 0, "Im(FFT)               ", FLT_MAX, FLT_MAX, { regAvail.x, regAvail.y / 2.f });
	}

	if (showRealPart) {
		ImGui::PushStyleColor(ImGuiCol_PlotLines, { 0.88f, 0.15f, 0.25f, 0.5f });
		ImGui::PushStyleColor(ImGuiCol_Text, { 0.88f, 0.15f, 0.25f, 0.6f });
		ImGui::SetCursorPos(pos);
		ImGui::PlotLines("##Re(FFT)", [](void* data, int idx) { return (float)waveFFT[idx].real(); }, nullptr, DATA_SIZE/2, 0, "\t\t\t\t\t Re(FFT)", -50, 50, { regAvail.x, regAvail.y / 2.f });
	}

	ImGui::PopStyleColor(showImaginaryPart * 2 + showRealPart * 2 + showPhaseShift * 2 + showAmplitude * 2 + 1);

	//ImGui::PlotLines("##PhaseShift", [](void* data, int idx) { return (std::abs(waveFFT_Cooley_Real[idx]) > (DATA_SIZE/2.f)) ? (float)std::arg(waveFFT_Cooley_Real[idx]) : 0; }, nullptr, DATA_SIZE/2, 0, "", -PI, PI, { regAvail.x, regAvail.y / 2.f });

	ImGui::EndGroup();

	ImGui::SameLine();


	return 0;
}

int main()
{
	// We do dynamic allocation cuz I can't imagine allocating ~1GB or memory statically
	wave = new std::complex<SCALAR>[DATA_SIZE];
	wave2 = new std::complex<SCALAR>[DATA_SIZE];
	wave3 = new std::complex<SCALAR>[DATA_SIZE];
	waveFFT = new std::complex<SCALAR>[DATA_SIZE];
	waveFFT_Cooley = new std::complex<SCALAR>[DATA_SIZE];
	waveFFT_Cooley_Real = new std::complex<SCALAR>[DATA_SIZE];

	GUI::Setup(OnGui);

	UpdateWaves();

	ImGui::GetIO().IniFilename = NULL;

	while (true) {
		if(GUI::DrawGui())
			break;
	}

	return 0;
}