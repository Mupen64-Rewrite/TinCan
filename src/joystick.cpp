#include "joystick.hpp"
#include <wx/event.h>
#include <wx/colour.h>
#include <wx/gdicmn.h>
#include <wx/pen.h>
#include <wx/settings.h>
#include <wx/validate.h>
#include <wx/dcclient.h>

namespace tasinput {
  namespace {
    
    /**
     * Guesses an alternate colour that contrasts slightly
     * with this colour.
     * @param col the colour involved
     * @returns a guessed alternate colour to use.
     */
    wxColour GuessAltColour(const wxColour& col) {
      if (col.GetLuminance() > 0.5)
        return col.ChangeLightness(90);
      else
        return col.ChangeLightness(110);
    }
  }
  
  Joystick::Joystick() {
    Init();
  }

  Joystick::Joystick(
    wxWindow* parent, wxWindowID winid, const wxPoint& pos, const wxSize& size,
    long style, const wxString& name) : Joystick() {
    Create(parent, winid, pos, size, style, name);
  }
  
  void Joystick::Create(
      wxWindow* parent, wxWindowID winid,
      const wxPoint& pos,
      const wxSize& size, long style,
      const wxString& name) {
    wxControl::Create(parent, winid, pos, size, style, wxDefaultValidator, name);
    this->Bind(wxEVT_PAINT, &Joystick::OnPaint, this);
  }
  
  wxSize Joystick::DoGetBestSize() const {
    return FromDIP(wxSize {256, 256});
  }
  
  void Joystick::Init() {
    posX = posY = 0;
  }
  
  void Joystick::OnPaint(wxPaintEvent&) {
    wxPaintDC dc(this);
    
    const auto colBG = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    const auto colBG2 = GuessAltColour(colBG);
    const auto colLines = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT);
    const auto colDot = wxSystemSettings::GetColour(wxSYS_COLOUR_HOTLIGHT);
    const auto colStick = colDot.ChangeLightness(80);
    
    const auto size = GetClientSize();
    
    int joyX = ((posX + 128) * size.x) / 256;
    int joyY = (256 - (posY + 128)) * size.y / 256;
    
    // background
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(colBG);
    dc.DrawRectangle(wxPoint {0, 0}, FromDIP(size));
    
    // circle
    dc.SetPen(wxPen(colLines, 1));
    dc.SetBrush(colBG2);
    dc.DrawEllipse(wxPoint {0, 0}, FromDIP(size));
    
    // crosshair
    dc.DrawLine(FromDIP(wxPoint {0, size.y / 2}), FromDIP(wxPoint {size.x, size.y / 2}));
    dc.DrawLine(FromDIP(wxPoint {size.x / 2, 0}), FromDIP(wxPoint {size.x / 2, size.y}));
    
    // joystick line
    dc.SetPen(wxPen(colStick, 3));
    dc.DrawLine(FromDIP(wxPoint {size.x / 2, size.y / 2}), FromDIP(wxPoint {joyX, joyY}));
    
    // joystick dot
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(colDot);
    dc.DrawCircle(FromDIP(wxPoint {joyX, joyY}), 5);
  }
  
  void Joystick::OnMouse(wxMouseEvent& evt) {
    
  }
}  // namespace tasinput