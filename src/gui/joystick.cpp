#include "joystick.hpp"
#include <wx/event.h>
#include <wx/colour.h>
#include <wx/gdicmn.h>
#include <wx/mousestate.h>
#include <wx/pen.h>
#include <wx/settings.h>
#include <wx/validate.h>
#include <wx/dcclient.h>

#include <algorithm>
#include <iostream>

namespace tasinput {
  
  // Event
  // ================
  
  wxDEFINE_EVENT(TASINPUT_EVT_JSCTRL, JoystickControlEvent);
  
  // Widget
  // ================
  
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
    Bind(wxEVT_PAINT, &Joystick::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &Joystick::OnMouse, this);
    Bind(wxEVT_MOTION, &Joystick::OnMouse, this);
  }
  
  BUTTONS Joystick::QueryState() {
    return BUTTONS {
      .R_DPAD = 0,
      .L_DPAD = 0,
      .D_DPAD = 0,
      .U_DPAD = 0,
      
      .START_BUTTON = 0,
      .Z_TRIG = 0,
      .B_BUTTON = 0,
      .A_BUTTON = 0,
      
      .R_CBUTTON = 0,
      .L_CBUTTON = 0,
      .D_CBUTTON = 0,
      .U_CBUTTON = 0,
      
      .R_TRIG = 0,
      .L_TRIG = 0,
      .Reserved1 = 0,
      .Reserved2 = 0,
      
      .X_AXIS = posX,
      .Y_AXIS = posY
    };
  }
  
  wxSize Joystick::DoGetBestSize() const {
    return FromDIP(wxSize {64, 64});
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
    if (!evt.ButtonIsDown(wxMOUSE_BTN_LEFT))
      return;
      
    posX = (evt.m_x * 256 / m_width) - 128;
    posY = (256 - (evt.m_y * 256 / m_height)) - 128;
    
    // "Dumb" clamping: coordinates are capped based on closest side
    posX = std::clamp(posX, -128, 127);
    posY = std::clamp(posY, -128, 127);
    
    // snap to axes
    if (-8 < posX && posX < 8) posX = 0;
    if (-8 < posY && posY < 8) posY = 0;
    
    // Fire an event
    auto* event = new JoystickControlEvent;
    event->SetValue({posX, posY});
    wxQueueEvent(this, event);
    
    Refresh();
  }
}  // namespace tasinput