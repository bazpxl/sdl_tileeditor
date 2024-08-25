

#ifndef GUI_H
#define GUI_H
#include "examplegame.h"

namespace BzlGame {

	class EditorState;
	class GUI
	{
	public:
		GUI(Map * map, EditorState * state);

		void RenderAll();
	private:

		Map * map_;
		EditorState * state_;


		int panel_slider_;
	#ifdef BZ_IMGUI_ACTIVE
		bool imgui_window_active = true;
	#endif

		void Menubar() const;



	};

} // BzlGame

#endif //GUI_H


