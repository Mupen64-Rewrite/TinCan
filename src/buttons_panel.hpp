//
// Created by jgcodes on 23/08/22.
//

#ifndef TASINPUT2_BUTTONS_PANEL_HPP_INCLUDED
#define TASINPUT2_BUTTONS_PANEL_HPP_INCLUDED

#include <wx/statbox.h>
#include <wx/button.h>
#include <wx/tglbtn.h>
#include <wx/stattext.h>
#include <mupen64plus/m64p_plugin.h>
#include <memory>

namespace tasinput {

  class ButtonsPanel : public wxStaticBox {
  public:
    ButtonsPanel(wxWindow* parent);

    BUTTONS QueryState();

  private:
    wxToggleButton* btnL;
    wxToggleButton* btnZ;
    wxToggleButton* btnR;

    wxStaticText* lblD;
    wxToggleButton* btnDU;
    wxToggleButton* btnDD;
    wxToggleButton* btnDL;
    wxToggleButton* btnDR;

    wxToggleButton* btnStart;
    wxToggleButton* btnB;
    wxToggleButton* btnA;

    wxStaticText* lblC;
    wxToggleButton* btnCU;
    wxToggleButton* btnCD;
    wxToggleButton* btnCL;
    wxToggleButton* btnCR;
  };

} // tasinput

#endif //TASINPUT2_BUTTONS_PANEL_HPP_INCLUDED
