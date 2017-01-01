#pragma once

#include <unordered_map>
#include "core/subsystem.h"
#include "input_mapping.hpp"

#include "core/common/basetypes.hpp"
#include "core/events/event.hpp"

using ActionMap = std::unordered_map<std::string, std::unordered_map<ActionType, event<void(const sf::Event&)>>>;

struct ActionMapper
{
	///
	KeyboardMapper keyboard_mapper;
	///
	MouseButtonMapper mouse_button_mapper;
	///
	MouseWheelMapper mouse_wheel_mapper;
	///
	TouchFingerMapper touch_finger_mapper;
	///
	JoystickButtonMapper joystick_mapper;
	///
	EventMapper event_mapper;
	///
	ActionMap actions;

	void handleEvent(const sf::Event& event)
	{
		auto triggerCallbacks = [this](auto mapper, const sf::Event& event)
		{
			auto mappings = mapper.getMapping(event);
			for (auto& action : mappings.actions)
			{
				auto& mappedEvent = actions[action][mappings.type];
				mappedEvent(event);
			}
		};

		triggerCallbacks(keyboard_mapper, event);
		triggerCallbacks(mouse_button_mapper, event);
		triggerCallbacks(mouse_wheel_mapper, event);
		triggerCallbacks(touch_finger_mapper, event);
		triggerCallbacks(joystick_mapper, event);
		triggerCallbacks(event_mapper, event);
	}
};

class Input : public core::Subsystem
{
public:
	Input();
	//-----------------------------------------------------------------------------
	//  Name : update ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	void update();

	//-----------------------------------------------------------------------------
	//  Name : handle_event ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	void handle_event(const sf::Event& event);

	//-----------------------------------------------------------------------------
	//  Name : is_key_pressed ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	bool is_key_pressed(sf::Keyboard::Key key);

	//-----------------------------------------------------------------------------
	//  Name : is_key_down ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	bool is_key_down(sf::Keyboard::Key key);

	//-----------------------------------------------------------------------------
	//  Name : is_key_released ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	bool is_key_released(sf::Keyboard::Key key);

	//-----------------------------------------------------------------------------
	//  Name : mouseMoved ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	inline bool mouse_moved() const { return _mouse_move_event; }

	//-----------------------------------------------------------------------------
	//  Name : get_cursor_position ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	inline const iPoint& get_current_cursor_position() const { return _current_cursor_position; }

	//-----------------------------------------------------------------------------
	//  Name : get_cursor_last_position ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	inline const iPoint& get_last_cursor_position() const { return _last_cursor_position; }

	//-----------------------------------------------------------------------------
	//  Name : get_cursor_delta_move ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	iPoint get_cursor_delta_move() const;

	//-----------------------------------------------------------------------------
	//  Name : is_mouse_button_pressed ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	bool is_mouse_button_pressed(sf::Mouse::Button button);

	//-----------------------------------------------------------------------------
	//  Name : is_mouse_button_down ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	bool is_mouse_button_down(sf::Mouse::Button button);

	//-----------------------------------------------------------------------------
	//  Name : is_mouse_button_released ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	bool is_mouse_button_released(sf::Mouse::Button button);

	//-----------------------------------------------------------------------------
	//  Name : is_mouse_wheel_scrolled ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	inline bool is_mouse_wheel_scrolled() const { return mouse_wheel_scrolled; }

	//-----------------------------------------------------------------------------
	//  Name : get_mouse_wheel_scroll ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	inline float get_mouse_wheel_scroll_delta_move() const { return _mouse_scroll_delta; }

	//-----------------------------------------------------------------------------
	//  Name : is_joystick_connected ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	bool is_joystick_connected(unsigned int joystickId);

	//-----------------------------------------------------------------------------
	//  Name : is_joystick_active ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	bool is_joystick_active(unsigned int joystickId);

	//-----------------------------------------------------------------------------
	//  Name : is_joystick_disconnected ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	bool is_joystick_disconnected(unsigned int joystickId);

	//-----------------------------------------------------------------------------
	//  Name : is_joystick_button_pressed ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	bool is_joystick_button_pressed(unsigned int joystickId, unsigned int button);

	//-----------------------------------------------------------------------------
	//  Name : is_joystick_button_down ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	bool is_joystick_button_down(unsigned int joystickId, unsigned int button);

	//-----------------------------------------------------------------------------
	//  Name : is_joystick_button_released ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	bool is_joystick_button_released(unsigned int joystickId, unsigned int button);

	//-----------------------------------------------------------------------------
	//  Name : get_joystick_axis_position ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	float get_joystick_axis_position(unsigned int joystickId, sf::Joystick::Axis axis);

private:
	//-----------------------------------------------------------------------------
	//  Name : key_update ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	void key_update();

	//-----------------------------------------------------------------------------
	//  Name : key_event ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	bool key_event(const sf::Event& event);

	//-----------------------------------------------------------------------------
	//  Name : mouseUpdate ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	void mouse_update();

	//-----------------------------------------------------------------------------
	//  Name : mouseEvent ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	bool mouse_event(const sf::Event& event);

	//-----------------------------------------------------------------------------
	//  Name : joystickUpdate ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	void joystick_update();

	//-----------------------------------------------------------------------------
	//  Name : joystickEvent ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	bool joystick_event(const sf::Event& event);

	///
	ActionMapper _action_mapper;
	/// 
	bool _mouse_move_event = false;
	/// 
	bool mouse_wheel_scrolled = false;
	/// 
	float _mouse_scroll_delta = 0.0f;
	/// 
	iPoint _current_cursor_position;
	/// 
	iPoint _last_cursor_position;
	/// 
	std::unordered_map<unsigned int, bool> _mouse_buttons_pressed;
	/// 
	std::unordered_map<unsigned int, bool> _mouse_buttons_down;
	/// 
	std::unordered_map<unsigned int, bool> _mouse_buttons_released;
	/// 
	std::unordered_map<unsigned int, bool> _keys_pressed;
	/// 
	std::unordered_map<unsigned int, bool> _keys_down;
	/// 
	std::unordered_map<unsigned int, bool> _keys_released;
	/// 
	std::unordered_map<unsigned int, bool> _joysticks_connected;
	/// 
	std::unordered_map<unsigned int, bool> _joysticks_active;
	/// 
	std::unordered_map<unsigned int, bool> _joysticks_disconnected;
	/// 
	std::unordered_map<std::pair<unsigned int, unsigned int>, bool> _joystick_buttons_pressed;
	/// 
	std::unordered_map<std::pair<unsigned int, unsigned int>, bool> _joystick_buttons_down;
	/// 
	std::unordered_map<std::pair<unsigned int, unsigned int>, bool> _joystick_buttons_released;
	/// 
	std::unordered_map<std::pair<unsigned int, unsigned int>, float> _joystick_axis_positions;
};