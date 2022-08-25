#ifndef TASINPUT2_GUI_JOYSTICK_PANEL_HPP_INCLUDED
#define TASINPUT2_GUI_JOYSTICK_PANEL_HPP_INCLUDED

#include <wx/control.h>
#include <wx/event.h>

#include <mupen64plus/m64p_plugin.h>
#include <wx/gdicmn.h>
#include <utility>

namespace tasinput {
  
  // Event thing
  // ================

  class JoystickControlEvent;
  wxDECLARE_EVENT(TASINPUT_EVT_JSCTRL, JoystickControlEvent);

  class JoystickControlEvent : public wxCommandEvent {
  public:
    JoystickControlEvent(wxEventType commandType = TASINPUT_EVT_JSCTRL, int id = 0) :
      wxCommandEvent(commandType, id) {}

    JoystickControlEvent(const JoystickControlEvent& event) :
      wxCommandEvent(event) {}

    wxEvent* Clone() const { return new wxCommandEvent(*this); }

    wxPoint GetValue() { return value; }
    void SetValue(const wxPoint& p) { value = p; }

  private:
    wxPoint value;
  };

  typedef void (wxEvtHandler::*JoystickControlEventFunction)(
    JoystickControlEvent&);

#define tasinputJoystickEventHandler(func) \
  wxEVENT_HANDLER_CAST(JoystickControlEventFunction, func);
  
  // Widget
  // ================

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

    BUTTONS QueryState();
    
    int GetPosX() { return posX; }
    void SetPosX(int value) { posX = value; }
    
    int GetPosY() { return posY; }
    void SetPosY(int value) { posY = value; }

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