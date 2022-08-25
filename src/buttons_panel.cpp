//
// Created by jgcodes on 23/08/22.
//

#include "buttons_panel.hpp"

#include <wx/anybutton.h>
#include <wx/gdicmn.h>
#include <wx/gbsizer.h>
#include <wx/generic/panelg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <initializer_list>

namespace tasinput {

  ButtonsPanel::ButtonsPanel(wxWindow* parent) :
    wxPanel(parent, wxID_ANY),
    szrRoot(new wxStaticBoxSizer(wxVERTICAL, this, "Buttons")),
    btnL(new wxToggleButton(szrRoot->GetStaticBox(), wxID_ANY, "L", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT)),
    btnZ(new wxToggleButton(szrRoot->GetStaticBox(), wxID_ANY, "Z", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT)),
    btnR(new wxToggleButton(szrRoot->GetStaticBox(), wxID_ANY, "R", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT)),

    lblD(new wxStaticText(szrRoot->GetStaticBox(), wxID_ANY, "D")),
    btnDU(new wxToggleButton(szrRoot->GetStaticBox(), wxID_ANY, "^", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT)),
    btnDD(new wxToggleButton(szrRoot->GetStaticBox(), wxID_ANY, "v", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT)),
    btnDL(new wxToggleButton(szrRoot->GetStaticBox(), wxID_ANY, "<", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT)),
    btnDR(new wxToggleButton(szrRoot->GetStaticBox(), wxID_ANY, ">", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT)),

    btnStart(new wxToggleButton(szrRoot->GetStaticBox(), wxID_ANY, "S", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT)),
    btnB(new wxToggleButton(szrRoot->GetStaticBox(), wxID_ANY, "B", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT)),
    btnA(new wxToggleButton(szrRoot->GetStaticBox(), wxID_ANY, "A", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT)),

    lblC(new wxStaticText(szrRoot->GetStaticBox(), wxID_ANY, "C")),
    btnCU(new wxToggleButton(szrRoot->GetStaticBox(), wxID_ANY, "^", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT)),
    btnCD(new wxToggleButton(szrRoot->GetStaticBox(), wxID_ANY, "v", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT)),
    btnCL(new wxToggleButton(szrRoot->GetStaticBox(), wxID_ANY, "<", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT)),
    btnCR(new wxToggleButton(szrRoot->GetStaticBox(), wxID_ANY, ">", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT)) {
      
    for (auto* i : {btnDU, btnDD, btnDL, btnDR, btnCU, btnCD, btnCL, btnCR, btnStart, btnB, btnA}) {
      auto currSize = i->GetSize();
      i->SetMinSize({currSize.GetHeight(), currSize.GetHeight()});
    }
    
    auto* gridSizer = new wxGridBagSizer(2, 2);
    gridSizer->SetFlexibleDirection(wxVERTICAL);
    gridSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_ALL);
    
    gridSizer->Add(btnL, {0, 0}, {1, 3}, wxEXPAND);
    gridSizer->Add(btnZ, {0, 3}, {1, 3}, wxEXPAND);
    gridSizer->Add(btnR, {0, 6}, {1, 3}, wxEXPAND);
    
    gridSizer->Add(lblD, {2, 1}, wxDefaultSpan, wxALIGN_CENTER | wxEXPAND);
    gridSizer->Add(btnDU, {1, 1}, wxDefaultSpan, wxEXPAND);
    gridSizer->Add(btnDD, {3, 1}, wxDefaultSpan, wxEXPAND);
    gridSizer->Add(btnDL, {2, 0}, wxDefaultSpan, wxEXPAND);
    gridSizer->Add(btnDR, {2, 2}, wxDefaultSpan, wxEXPAND);
    
    gridSizer->Add(btnStart, {2, 4}, wxDefaultSpan, wxEXPAND);
    gridSizer->Add(btnB, {4, 5}, wxDefaultSpan, wxEXPAND);
    gridSizer->Add(btnA, {5, 6}, wxDefaultSpan, wxEXPAND);
    
    gridSizer->Add(lblC, {2, 7}, wxDefaultSpan, wxALIGN_CENTER | wxEXPAND);
    gridSizer->Add(btnCU, {1, 7}, wxDefaultSpan, wxEXPAND);
    gridSizer->Add(btnCD, {3, 7}, wxDefaultSpan, wxEXPAND);
    gridSizer->Add(btnCL, {2, 6}, wxDefaultSpan, wxEXPAND);
    gridSizer->Add(btnCR, {2, 8}, wxDefaultSpan, wxEXPAND);
    
    for (auto* i : {btnDU, btnDD, btnDL, btnDR, btnCU, btnCD, btnCL, btnCR, btnStart, btnB, btnA}) {
      gridSizer->SetItemMinSize(i, i->GetMinSize());
    }
    
    for (int i = 0; i < gridSizer->GetCols(); i++)
      gridSizer->AddGrowableCol(i, 1);
    for (int i = 0; i < gridSizer->GetRows(); i++)
      gridSizer->AddGrowableRow(i, 1);
    
    szrRoot->Add(gridSizer, 1, wxEXPAND | wxALL, 4);
    SetSizerAndFit(szrRoot);
  }

  BUTTONS ButtonsPanel::QueryState() {
    return BUTTONS {
      .R_DPAD = btnDR->GetValue(),
      .L_DPAD = btnDL->GetValue(),
      .D_DPAD = btnDD->GetValue(),
      .U_DPAD = btnDU->GetValue(),
      
      .START_BUTTON = btnStart->GetValue(),
      .Z_TRIG = btnZ->GetValue(),
      .B_BUTTON = btnB->GetValue(),
      .A_BUTTON = btnA->GetValue(),
      
      .R_CBUTTON = btnDR->GetValue(),
      .L_CBUTTON = btnDL->GetValue(),
      .D_CBUTTON = btnDD->GetValue(),
      .U_CBUTTON = btnDU->GetValue(),
      
      .R_TRIG = btnR->GetValue(),
      .L_TRIG = btnL->GetValue(),
      .Reserved1 = 0,
      .Reserved2 = 0,
      
      .X_AXIS = 0,
      .Y_AXIS = 0
    };
  }
}  // namespace tasinput