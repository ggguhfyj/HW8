/**
 * \file
 * \author  JUNSEOK LEE
 * \author Jonathan Holmes
 * \author Junseok Your Name
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */

#pragma once
#include <gsl/gsl>
#include <vector>
#include "SDL_stdinc.h"
//#include <SDL2/SDL_scancode.h>
#include <SDL_scancode.h>
namespace CS230
{
    class Input
    {
    public:
        enum class Keys
        {
            A,
            B,
            C,
            D,
            E,
            F,
            G,
            H,
            I,
            J,
            K,
            L,
            M,
            N,
            O,
            P,
            Q,
            R,
            S,
            T,
            U,
            V,
            W,
            X,
            Y,
            Z,
            One,
            Two,
            Three,
            Four,
            Numpad4,
            Numpad5,
            Numpad6,
            Numpad8,
            Space,
            Enter,
            Left,
            Up,
            Right,
            Down,
            Escape,
            Tab,
            Count
        };

        Input();
        void Init();
        void Update();

        bool KeyDown(Keys key) const;
        bool KeyJustReleased(Keys key) const;
        bool KeyJustPressed(Keys key) const;
        SDL_Scancode convert_cs230_to_sdl(Keys cs230_key);
        
    private:
        std::vector<bool> keys_down;
        std::vector<bool> previous_keys_down;

    private:
        void SetKeyDown(Keys key, bool value);
    };

    constexpr Input::Keys& operator++(Input::Keys& the_key) noexcept
    {
        the_key = static_cast<Input::Keys>(static_cast<unsigned>(the_key) + 1);
        return the_key;
    }

    constexpr gsl::czstring to_string(Input::Keys key) noexcept
    {
        switch (key)
        {
            case Input::Keys::A: return "A";
            case Input::Keys::B: return "B";
            case Input::Keys::C: return "C";
            case Input::Keys::D: return "D";
            case Input::Keys::E: return "E";
            case Input::Keys::F: return "F";
            case Input::Keys::G: return "G";
            case Input::Keys::H: return "H";
            case Input::Keys::I: return "I";
            case Input::Keys::J: return "J";
            case Input::Keys::K: return "K";
            case Input::Keys::L: return "L";
            case Input::Keys::M: return "M";
            case Input::Keys::N: return "N";
            case Input::Keys::O: return "O";
            case Input::Keys::P: return "P";
            case Input::Keys::Q: return "Q";
            case Input::Keys::R: return "R";
            case Input::Keys::S: return "S";
            case Input::Keys::T: return "T";
            case Input::Keys::U: return "U";
            case Input::Keys::V: return "V";
            case Input::Keys::W: return "W";
            case Input::Keys::X: return "X";
            case Input::Keys::Y: return "Y";
            case Input::Keys::Z: return "Z";
            case Input::Keys::One: return "1";
            case Input::Keys::Two: return "2";
            case Input::Keys::Three: return "3";
            case Input::Keys::Four: return "4";
            case Input::Keys::Numpad4: return "Numpad4";
            case Input::Keys::Numpad5: return "Numpad5";
            case Input::Keys::Numpad6: return "Numpad6";
            case Input::Keys::Numpad8: return "Numpad8";
            case Input::Keys::Space: return "Space";
            case Input::Keys::Enter: return "Enter";
            case Input::Keys::Left: return "Left";
            case Input::Keys::Up: return "Up";
            case Input::Keys::Right: return "Right";
            case Input::Keys::Down: return "Down";
            case Input::Keys::Escape: return "Escape";
            case Input::Keys::Tab: return "Tab";
            case Input::Keys::Count: return "Count";
        }
        return "Unknown";
    }
}
