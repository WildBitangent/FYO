#include "GUI.hpp"
#include <d3d11.h>
#include "ImGUI/imgui.h"
#include "IMGUI/imgui_impl_dx11.h"
#include "ImGUI/imgui_impl_win32.h"
#include "spdlog/fmt/fmt.h"
//#include "Input.hpp"
#include "Message.hpp"
#include "Constants.hpp"
#include "Logic.hpp"
#include "Util.hpp"

LensGUI::LensGUI()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	[[maybe_unused]]
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = { WIDTH, HEIGHT };

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	MessageBus::postExpress({ MessageID::GUI_INIT, {.datap = this} });
}

LensGUI::~LensGUI()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void LensGUI::init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
{
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(device, context);

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = { WIDTH, HEIGHT };
}

void LensGUI::update(Lens& lens, const std::vector<Sample>& samples)
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (!mSelectedSample)
		mSelectedSample = &samples.front();

	mFPSHistory.emplace(ImGui::GetIO().Framerate);

	{
		ImGui::Begin("Settings");
		ImGui::PlotLines("FPS", mFPSHistory.data().data(), mFPSHistory.data().size(), 0, nullptr, 0.f, 144.f);
		ImGui::Separator();
		ImGui::PushItemWidth(150);

		////////////////////// Settings //////////////////////
		{
			if (ImGui::DragInt("Ortogonal Beam Rays Count", reinterpret_cast<int*>(&mBeamCount), 0.05f, 3, 32))// TODO is this really ortogonal
				MessageBus::post({ MessageID::LOGIC, Logic::MessageID::UPDATE_BEAM });

			if (ImGui::DragFloat("Ortogonal Beam Radius", &mBeamRadius, 0.01f, 0.025f, 10.f)) // TODO is this really ortogonal
				MessageBus::post({ MessageID::LOGIC, Logic::MessageID::UPDATE_BEAM });

			//if (ImGui::DragInt("Camera Beam Rays Count", reinterpret_cast<int*>(&mRaysCount), 0.05f, 3, 32))
			//	MessageBus::post({ MessageID::LOGIC, Logic::MessageID::UPDATE_BEAM });
			//
			//if (ImGui::DragFloat("Camera Beam Radius", &mRaysRadius, 0.001f, 0.025f, 0.5f))
			//	MessageBus::post({ MessageID::LOGIC, Logic::MessageID::UPDATE_BEAM });

			if (ImGui::DragFloat("Ray Width", &lens.getConstBuff().lineWidth, 0.0005f, 0.0025f, 0.15f))
				MessageBus::post({ MessageID::LOGIC, Logic::MessageID::UPDATE_CONST_BUF });

			if (ImGui::DragFloat("IOR", &lens.getConstBuff().lensIOR, 0.001f, 0.5f, 2.5f))
				MessageBus::post({ MessageID::LOGIC, Logic::MessageID::UPDATE_CONST_BUF });

			ImGui::SameLine();
			if (ImGui::Checkbox("Chromatic Aberration", reinterpret_cast<bool*>(&lens.getConstBuff().chromaticAberration)))
				MessageBus::post({ MessageID::LOGIC, Logic::MessageID::UPDATE_CONST_BUF });
		}
		ImGui::Separator();

		////////////////////// LENS COMBO //////////////////////
		if (ImGui::BeginCombo("Lens", mSelectedLens ? *mSelectedLens : (lens.data().empty() ? "" : lens.data().front())))
		{
			for (auto& l : lens.data())
			{
				ImGui::PushID(&l);
				if (ImGui::Selectable(l, &l == mSelectedLens))
					mSelectedLens = &l;
				ImGui::PopID();
			}

			ImGui::EndCombo();
		}

		////////////////////// SAMPLES COMBO //////////////////////
		if (ImGui::BeginCombo("Samples", mSelectedSample->name.c_str())) 
		{
			for (auto& s : samples)
			{
				ImGui::PushID(&s);
				if (ImGui::Selectable(s.name.c_str(), &s == mSelectedSample))
				{
					mSelectedSample = &s;
					mSelectedLens = nullptr;
					s.funct(lens);
					MessageBus::post({ MessageID::LOGIC, Logic::MessageID::UPDATE_LENS });
				}
				ImGui::PopID();
			}

			ImGui::EndCombo();
		}

		////////////////////// LENS EDITOR //////////////////////
		ImGui::SameLine();
		if (ImGui::Button("Add Lens"))
		{
			LensStruct lll;
			lens.push(lll);
			mSelectedLens = &lens.data().back();
			MessageBus::post({ MessageID::LOGIC, Logic::MessageID::UPDATE_LENS });
		}

		ImGui::Separator();
		ImGui::Text(
			"Movement WSAD + rotation with RMB\n"
			"C - Parallel vertical rays\n"
			"B - Parallel spherical rays\n"
			"V - Rays coming out from camera\n"
		);

		if (mSelectedLens)
			ImGui::OpenPopup("Lens Edit");

		bool openLensEditor = true;
		ImGui::SetNextWindowSizeConstraints(ImVec2(320, -1), ImVec2(320, -1));
		if (ImGui::BeginPopupModal("Lens Edit", &openLensEditor))
		{
			////////////////////// TYPE //////////////////////
			if (ImGui::BeginCombo("Type", *mSelectedLens))
			{
				for (size_t i = 0; i < static_cast<size_t>(LensType::COUNT); i++)
					if (auto type = static_cast<LensType>(i); ImGui::Selectable(lensToString(type), type == mSelectedLens->type))
						mSelectedLens->type = type;
				
				ImGui::EndCombo();
			}

			////////////////////// RADIUS //////////////////////
			ImGui::DragFloat("Radius 1", &mSelectedLens->radius1, 0.01f, 3.f, 300.f);

			if (mSelectedLens->type != LensType::PLANOCONVEX)
			{
				//ImGui::SameLine();
				ImGui::DragFloat("Radius 2", &mSelectedLens->radius2, 0.01f, 3.f, 300.f);
			}

			////////////////////// CENTER //////////////////////
			ImGui::DragFloat3("Center", reinterpret_cast<float*>(&mSelectedLens->center), 0.01f); 
			ImGui::DragFloat("Width", &mSelectedLens->width, 0.01f, 0.01f, 25.f);
			ImGui::DragFloat2("Size", reinterpret_cast<float*>(&mSelectedLens->dimensions), 0.01f, 3.f, 300.f);
			
			ImGui::Spacing();
			ImGui::SameLine(130);
			if (ImGui::Button("Delete"))
			{
				ImGui::CloseCurrentPopup();
				lens.erase(*mSelectedLens);
				mSelectedLens = nullptr;
			}
			else // todo It's not necessary to update every frame
				mSelectedLens = &lens.updateLens(*mSelectedLens);
			
			MessageBus::post({ MessageID::LOGIC, Logic::MessageID::UPDATE_LENS });
			
			ImGui::EndPopup();
		}

		if (!openLensEditor)
			mSelectedLens = nullptr;

		ImGui::End();
	}
}

void LensGUI::render() const
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}