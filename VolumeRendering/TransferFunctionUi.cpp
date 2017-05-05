#include "TransferFunctionUi.h"
#include "RaycastVolume.h"
#include "CinderImGui.h"
using namespace glm;
using namespace ci;

void TransferFunctionUi::drawThresholdControl()
{
    // draw threshold indicator
    const ImVec2 p = ui::GetCursorScreenPos();
    ImDrawList* drawList = ui::GetWindowDrawList();
    auto width = ui::GetContentRegionAvailWidth();
    ImU32 gray = ImColor(0.5f, 0.5f, 0.5f, 0.5f);
    float x = p.x;
    float y = p.y - 35.0f - 4.0f;
    float steps = width / 255;
    ImGuiStyle& style = ui::GetStyle();
    auto threshold = getThreshold();

    if (threshold.x > 0)
    {
        drawList->AddRectFilled(ImVec2(x, y - 120), ImVec2(x + threshold.x * steps, y), gray, style.FrameRounding);
    }

    if (threshold.y < 255)
    {
        drawList->AddRectFilled(ImVec2(x + threshold.y * steps, y - 120), ImVec2(x + 255 * steps, y), gray, style.FrameRounding);
    }

    // draw threshold controls
    ui::BeginGroup();
    ui::PushItemWidth(-2.0f);

    if (ui::SliderInt2("##Threshold", value_ptr(threshold), 0, 255))
    {
        setThreshold(threshold.x, threshold.y);
        updateFunction();
    }

    ui::PopItemWidth();
    ui::EndGroup();
}

void TransferFunctionUi::drawUi(bool& open, const RaycastVolume& volume)
{
    if (open)
    {
        ui::Begin("Transfer Function", &open, ImGuiWindowFlags_AlwaysAutoResize);
        drawHistogram(volume);
        drawControlPointsUi();
        drawThresholdControl();
        drawControlPointCreationUi();
        ui::End();
    }
}

TransferFunctionUi::TransferFunctionUi() {}

TransferFunctionUi::~TransferFunctionUi() {}

void TransferFunctionUi::drawHistogram(const RaycastVolume& volume) const
{
    ui::BeginGroup();
    auto& histogram = volume.getHistogram();
    ui::PlotHistogram("##histogram", histogram.data(), histogram.size(), 0, nullptr, 0.0f, 1.0f, ImVec2(520, 120));
    ui::EndGroup();
}

void TransferFunctionUi::drawControlPointsUi()
{
    const ImVec2 p = ui::GetCursorScreenPos();
    ImDrawList* drawList = ui::GetWindowDrawList();
    int width = ui::GetContentRegionAvailWidth() - 4.0f;
    ImU32 white = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
    ImU32 black = ImColor(0.0f, 0.0f, 0.0f, 1.0f);
    float sz = 20.0f;
    float x = p.x + 4.0f;
    float y = p.y + 4.0f;
    float step = static_cast<float>(width) / 256;
    auto& colorPoints = getColorPoints();
    auto& alphaPoints = getAlphaPoints();

    for (int i = 0; i < 256; i++)
    {
        const auto& color = getColor(i);
        ImU32 col32 = ImColor(color);
        // draw color transfer function
        drawList->AddLine(ImVec2(x, y), ImVec2(x, y + sz), col32, 2.0f * step);
        x += step;
    }

    for (auto& colorP : colorPoints)
    {
        const auto& color = colorP.getColor();
        ImU32 col32 = ImColor(color.r, color.g, color.b, 1.0f);
        ImU32 invCol32 = ImColor(1.0f - color.r, 1.0f - color.g, 1.0f - color.b, 1.0f);
        drawList->AddCircleFilled(ImVec2(p.x + step * colorP.getIsoValue() + 4.0f, y + sz), 5, col32, 24);
        drawList->AddCircle(ImVec2(p.x + step * colorP.getIsoValue() + 4.0f, y + sz), 5, invCol32, 24);
    }

    for (int i = 0; i < 255; i++)
    {
        auto aBegin = clamp(1.0f - getColor(i).a, 0.0f, 1.0f);
        auto aEnd = clamp(1.0f - getColor(i + 1).a, 0.0f, 1.0f);
        drawList->AddLine(ImVec2(p.x + step * i + 4.0f, y - 124.0f + aBegin * 120),
            ImVec2(p.x + step * (i + 1) + 4.0f, y - 124.0f + aEnd * 120), white, 2);
    }

    for (auto& alphaP : alphaPoints)
    {
        const auto& alpha = 1.0f - alphaP.getAlpha();
        ImU32 opacity = ImColor(1.0f - vec4(alpha));
        drawList->AddCircleFilled(ImVec2(p.x + step * alphaP.getIsoValue() + 4.0f, y - 124.0f + alpha * 120), 5, opacity, 24);
        drawList->AddCircle(ImVec2(p.x + step * alphaP.getIsoValue() + 4.0f, y - 124.0f + alpha * 120), 5, black, 24);
    }

    ui::Dummy(ImVec2(width, sz + 15.0f));
}

void TransferFunctionUi::drawControlPointList(int pointType)
{
    bool deleteCtrlPoint = false;

    auto isoValueControl = [&](const TransferFunctionPoint& p, const int index)
    {
        int pIso = p.getIsoValue();
        ui::SameLine();
        ui::PushItemWidth(-30);

        if (p.getIsoValue() != 0 && p.getIsoValue() != 255)
        {
            if (ui::SliderInt("##isoValA", &pIso, 1, 254))
            {
                pIso = min(max(pIso, 1), 254);
                pointType == 0 ? setAlphaPointIsoValue(index, pIso)
                    : setColorPointIsoValue(index, pIso);
            }
        }
        else if (p.getIsoValue() == 0 || p.getIsoValue() == 255)
        {
            ui::PushStyleColor(ImGuiCol_Button, ImVec4(vec4(0.5f)));
            ui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(vec4(0.5f)));
            ui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(vec4(0.5f)));
            ui::Button(std::to_string(p.getIsoValue()).c_str(), ImVec2(ui::GetContentRegionAvailWidth() - 30, 0));
            ui::PopStyleColor(3);
        }

        ui::PopItemWidth();

        if (p.getIsoValue() != 0 && p.getIsoValue() != 255)
        {
            ui::SameLine();

            if (ui::Button("X", ImVec2(ui::GetContentRegionAvailWidth(), 0)))
            {
                deleteCtrlPoint = true;
            }
        }
    };

    // draw control point list
    if (pointType == 0 ? ui::TreeNode("Alpha Points") : ui::TreeNode("Color Points"))
    {
        ui::BeginGroup();
        ui::BeginChild(ui::GetID((void*)(intptr_t)pointType), ImVec2(ui::GetContentRegionAvailWidth(), 120), true);

        if (pointType == 0)
        {
            for (auto i = 0; i < getAlphaPoints().size(); i++)
            {
                auto& alphaP = getAlphaPoints()[i];
                auto pAlpha = alphaP.getAlpha();

                ui::PushID(&alphaP);
                ui::PushItemWidth(ui::GetContentRegionAvailWidth() * 0.5f);

                if (ui::DragFloat("##alpha", &pAlpha, 0.01f, 0.0f, 1.0f))
                {
                    setAlpha(i, pAlpha);
                }

                ui::PopItemWidth();
                isoValueControl(alphaP, i);

                if (deleteCtrlPoint)
                {
                    removeAlphaPoint(i--);
                    deleteCtrlPoint = false;
                    updateFunction();
                }

                ui::PopID();
            }
        }
        else if (pointType == 1)
        {
            for (auto i = 0; i < getColorPoints().size(); i++)
            {
                auto& colorP = getColorPoints()[i];
                auto pColor = colorP.getColor();

                ui::PushID(&colorP);
                ui::PushItemWidth(ui::GetContentRegionAvailWidth() * 0.5f);

                if (ui::ColorEdit3("##color", value_ptr(pColor)))
                {
                    setColor(i, pColor);
                }

                ui::PopItemWidth();
                isoValueControl(colorP, i);

                if (deleteCtrlPoint)
                {
                    removeColorPoint(i--);
                    deleteCtrlPoint = false;
                    updateFunction();
                }

                ui::PopID();
            }
        }

        ui::EndChild();
        ui::EndGroup();
        ui::TreePop();
    }
}

void TransferFunctionUi::drawControlPointCreationUi()
{
    // creation bar
    static int pointType = 0;
    static int isoValue = 127;
    static float alpha;
    static vec3 color;
    ui::BeginGroup();
    ui::Separator();
    ui::RadioButton("Alpha Point", &pointType, 0);
    ui::SameLine();
    ui::RadioButton("Color Point", &pointType, 1);
    // iso value bar
    ui::SliderInt("Iso Value", &isoValue, 1, 254);

    // alpha point
    if (pointType == 0)
    {
        ui::DragFloat("Alpha", &alpha, 0.01f, 0.0f, 1.0f);
    }
    else // color point
    {
        ui::ColorEdit3("Color", value_ptr(color));
    }

    ui::EndGroup();
    ImVec2 size = ImGui::GetItemRectSize();
    ui::SameLine();

    if (ui::Button("Add Point", ImVec2(ui::GetContentRegionAvailWidth(), size.y)))
    {
        pointType == 0 ? addAlphaPoint(alpha, isoValue) : addColorPoint(color, isoValue);
    }

    drawControlPointList(pointType);
}