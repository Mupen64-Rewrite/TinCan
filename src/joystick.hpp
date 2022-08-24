#ifndef TASINPUT2_JOYSTICK_PANEL_HPP_INCLUDED
#define TASINPUT2_JOYSTICK_PANEL_HPP_INCLUDED

#include <wx/control.h>
#include <wx/event.h>
#include <wx/rtti.h>

namespace tasinput {
  class Joystick : public wxControl {
  public:
    Joystick();

    Joystick(
      wxWindow* parent, wxWindowID winid,
      const wxPoint& pos = wxDefaultPosition,
      const wxSize& size = wxDefaultSize, long style = 0,
      const wxString& name = "tasinput::Joystick");

    void Create(
      wxWindow* parent, wxWindowID winid,
      const wxPoint& pos = wxDefaultPosition,
      const wxSize& size = wxDefaultSize, long style = 0,
      const wxString& name = "tasinput::Joystick");

  protected:
    void Init();
    
    wxSize DoGetBestSize() const override;
    
    void OnPaint(wxPaintEvent& event);
    void OnMouse(wxMouseEvent& event);
    
  private:
    int32_t posX;
    int32_t posY;
  };
}  // namespace tasinput

#endif