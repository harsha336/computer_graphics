# pragma once

# include <sig/sn_poly_editor.h>
# include <sig/sn_lines2.h>

# include <sigogl/ui_button.h>
# include <sigogl/ws_viewer.h>


#include <sig/sn_primitive.h>
#include <sig/sn_transform.h>
#include <sig/sn_manipulator.h>

# include <sigogl/ws_run.h>
#include<sigogl/ui_input.h>
#include<sigogl/ui_output.h>

// Viewer for this example:
class MyViewer : public WsViewer
{  protected :
	enum MenuEv { EvNormals, EvAnimate, EvExit };
	enum LinkId { palm, finger1, finger2};
	enum endEffector { ee1, ee2 };
	UiCheckButton* _nbut;
	bool _animating;
	GsVec links[3][5]; 
	//GsPnt targetP; 
	GsPnt originP; 
	int joints[3]; 
	float totalLength[3];
	float linkLength[3][4];
	SnLines *link_lines;
	
	SnPrimitive* p1;
	SnPrimitive* p2;
	SnGroup *g;
	GsVec efr[2];
   public :
	   SnManipulator *manip1;
	   SnManipulator *manip2;
	MyViewer ( int x, int y, int w, int h, const char* l );
	void update(GsPnt targetP,int eid);
	int fabrik(GsPnt targetP,int part);
	int fabrik_backward(GsPnt targetP,int part);
	int fabrik_forward(GsVec beginP,int part);
	void draw_links(int partChanged);
	float calculateDistance(GsVec a, GsVec b);
	void add_ui ();
	void add_model ( SnShape* s, GsVec p, SnManipulator *manip );
	void add_fingers(GsVec bot_center,GsVec top_center,SnGroup *g);
	void build_scene ();
	void show_normals ( bool b );
	void run_animation ();
	virtual int handle_keyboard ( const GsEvent &e ) override;
	virtual int uievent ( int e ) override;
	//SnPrimitive getPrimitive(int i) { return p[i]; }
	//SnPrimitive* getPrimitive2() { return p2; }
	GsVec getEe(int i) { return efr[i]; }
	void MyViewer::adjustManipulator(int e);
};

