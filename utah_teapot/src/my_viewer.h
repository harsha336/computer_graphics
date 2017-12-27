# pragma once

# include <sig/sn_poly_editor.h>
# include <sig/sn_lines2.h>
#include <sig/sn_primitive.h>
#include <sig/sn_transform.h>
#include <sig/sn_manipulator.h>
# include <sigogl/ui_button.h>
# include <sigogl/ws_viewer.h>
# include <sigogl/ws_run.h>
#include<sigogl/ui_input.h>
#include<sigogl/ui_output.h>
using namespace std;

// Viewer for this example:
class MyViewer : public WsViewer
{  protected :
	enum MenuEv { EvExit, EvBeizier, EvControlPoly, EvManipControlPoly, EvResol10, EvResol30, EvResol50, EvResol100};
   public :
	
	enum calledfrom {draw,man_start,man_comp};
	float bn(float t, int i);
	MyViewer ( int x, int y, int w, int h, const char* l );
	void set_new_control_points(int patch, int i, int j, GsPnt p);
	void add_ui ();
	void buildTeapot(bool normflag, int resol);
	void buildControlPolygons(int startedby);
	void makeNormals(GsModel *m, int resol);
	virtual int handle_keyboard ( const GsEvent &e ) override;
	virtual int uievent ( int e ) override;
	UiCheckButton* _viewbut[4];
	UiPanel* _spanel;
	int _spanelcmds;
	float control_poly[32][4][4][3];
};

