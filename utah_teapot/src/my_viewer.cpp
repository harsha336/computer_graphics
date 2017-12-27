
# include "my_viewer.h"
# include "sn_circle.h"
# include "glr_circle.h"
#include <sig/sn_primitive.h>
#include <sig/sn_transform.h>
#include <sig/sn_manipulator.h>
# include "glr_circle.h"
#include <fstream>
# include <sigogl/ui_button.h>
# include <sigogl/ws_run.h>
#include<sigogl/ui_slider.h>
#include<sigogl/ui_input.h>
#include<sigogl/ui_output.h>

# include <sigogl/ui_check_button.h>
# include <sigogl/ui_radio_button.h>
# include <sigogl/ui_color_chooser.h>

#include <string.h>
# include <sigogl/ui_dialogs.h>
using namespace std;

static bool normflag;
static bool controlPoly;
static int resolu;
static void callBack(SnManipulator* manip, const GsEvent& e, void* udata)
{
	MyViewer* v = (MyViewer*)udata;
	GsMat& m = manip->mat();
	float pnt[3];
	m.getrans(pnt[0], pnt[1], pnt[2]);
	GsPnt clicked_point(pnt);
	
	for (int patch = 0; patch < 32; patch++) {
		bool comp = 0;
		GsPnt control_point;
		int indexi, indexj;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				control_point.set(v->control_poly[patch][i][j][0], v->control_poly[patch][i][j][1], v->control_poly[patch][i][j][2]);

				if (control_point == clicked_point) {
					comp = 1;
					indexi = i; indexj = j;
				}
			}
		}	
		if (comp) {
			v->set_new_control_points(patch, indexi, indexj, control_point);
		}
	}	
}

MyViewer::MyViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	add_ui ();
	normflag = 0;
	controlPoly = 0;
	resolu = 10;
	ifstream instream;
	float num;
	instream.open("teapotCGA.bpt.txt");
	if (!instream) {
		gsout << "file does not exist or is unable to open";
		exit(1);
	}
	float arr[1536];
	int count = 0;
	while (instream >> num) {
		if (num != 3 && num != 32) {
			arr[count++] = num;
		}
	}
	int ind = 0;
	for (int patch = 0; patch < 32; patch++) {
		for (int i = 0; i <= 3; i++) {
			for (int j = 0; j <= 3; j++) {
				control_poly[patch][i][j][0] = arr[ind++];
				control_poly[patch][i][j][1] = arr[ind++];
				control_poly[patch][i][j][2] = arr[ind++];
				GsPnt p(control_poly[patch][i][j][0], control_poly[patch][i][j][1], control_poly[patch][i][j][2]);
			}
		}
	}
}

void MyViewer::set_new_control_points(int patch, int i, int j,GsPnt p) {
	control_poly[patch][i][j][0] = p(0);
	control_poly[patch][i][j][1] = p(1);
	control_poly[patch][i][j][2] = p(2);
	GsPnt cp(control_poly[patch][i][j][0], control_poly[patch][i][j][1], control_poly[patch][i][j][2]);
	buildControlPolygons(man_comp);
}

void MyViewer::add_ui ()
{
	UiPanel *p;
	UiPanel *sp;
	UiManager* uim = WsWindow::uim();
	p = uim->add_panel ( "", UiPanel::HorizLeft );
	p->add(new UiButton("Teapot", EvBeizier));
	p->add(new UiButton("ControlPoly", EvControlPoly));
	p->add(new UiButton("Manipulate", EvManipControlPoly));
	p->add ( new UiButton ( "Exit", EvExit ) );
	p->add(new UiButton("Resolution", sp = new UiPanel()));
	{	UiPanel* p = sp;
		UiStyle::StyleType t = UiStyle::Current.type;
		p->add(new UiRadioButton("Resol - 10", EvResol10));
		p->add(new UiRadioButton("Resol - 30", EvResol30));
		p->add(new UiRadioButton("Resol - 50", EvResol50));
		p->add(new UiRadioButton("Resol - 100", EvResol100));
	}
}

int MyViewer::handle_keyboard ( const GsEvent &e )
{
	int ret = WsViewer::handle_keyboard ( e ); // 1st let system check events
	if ( ret ) return ret;

	switch ( e.key )
	{	case GsEvent::KeyEsc : gs_exit(); return 1;
		default: gsout<<"Key pressed: "<<e.key<<gsnl;
	}
	
	return 0;
}

int MyViewer::uievent ( int e )
{
	switch ( e )
	{
		case EvBeizier: 
		{	if (normflag) {
				buildTeapot(1, resolu);
				normflag = !normflag;
			}
			else {
				buildTeapot(0, resolu);
				normflag = !normflag;
			}
			if (controlPoly) {
				buildControlPolygons(draw);
			}
			return 1;
		}	break;
		case EvControlPoly: 
			if (controlPoly) {
				if (normflag) {
					buildTeapot(0, resolu);
					controlPoly = !controlPoly;
				}
				else {
					buildTeapot(1, resolu);
					controlPoly = !controlPoly;
				}
			}
			else {
				buildControlPolygons(draw);
				controlPoly = !controlPoly;
			}
			return 1;
		case EvManipControlPoly: 
			buildTeapot(0, resolu);
			buildControlPolygons(man_start); 
			return 1;
		case EvResol10:
			resolu = 10;
			return 1;
		case EvResol30:
			resolu = 30;
			return 1;
		case EvResol50:
			resolu = 50;
			return 1;
		case EvResol100:
			resolu = 100;
			return 1;
		case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}

void MyViewer::buildControlPolygons(int startedby) {
	double t1, t2;
	int num_lines = 0;
	t1 = gs_time();
	SnLines *act_line;
	act_line = new SnLines;
	
	for (int patch = 0; patch < 32; patch++) {
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				SnManipulator* manip = new SnManipulator;
				GsMat m;
				SnLines *control_poly_line;
				control_poly_line = new SnLines;
				GsPnt p1(control_poly[patch][i][j][0], control_poly[patch][i][j][1], control_poly[patch][i][j][2]);
				GsPnt p2(control_poly[patch][i][j + 1][0], control_poly[patch][i][j + 1][1], control_poly[patch][i][j + 1][2]);
				GsPnt p3(control_poly[patch][i + 1][j][0], control_poly[patch][i + 1][j][1], control_poly[patch][i + 1][j][2]);
				m.translation(p1);
				manip->initial_mat(m);
				manip->visible(1);
				manip->callback(callBack, this);
				control_poly_line->push(p1, p2);
				control_poly_line->push(p1, p3);
				act_line->push(p1, p2);
				num_lines++;
				act_line->push(p1, p3);
				num_lines++;
				if (startedby == man_start) {
					manip->child(control_poly_line);
					rootg()->add(manip);
				}
			}
		}
		
		for (int i = 0; i < 3; i++) {
			SnManipulator* manip = new SnManipulator;
			GsMat m;
			SnLines *control_poly_line;
			control_poly_line = new SnLines;
			GsPnt p1(control_poly[patch][3][i][0], control_poly[patch][3][i][1], control_poly[patch][3][i][2]);
			GsPnt p2(control_poly[patch][3][i + 1][0], control_poly[patch][3][i + 1][1], control_poly[patch][3][i + 1][2]);
			m.translation(p1);
			manip->initial_mat(m);
			manip->visible(1);
			manip->callback(callBack, this);
			control_poly_line->push(p1, p2);
			act_line->push(p1, p2);
			num_lines++;
			if (startedby == man_start) {
				manip->child(control_poly_line);
				rootg()->add(manip);
			}
		}
		for (int i = 0; i < 3; i++) {
			SnManipulator* manip = new SnManipulator;
			GsMat m;
			SnLines *control_poly_line;
			control_poly_line = new SnLines;
			GsPnt p1(control_poly[patch][i][3][0], control_poly[patch][i][3][1], control_poly[patch][i][3][2]);
			GsPnt p2(control_poly[patch][i + 1][3][0], control_poly[patch][i + 1][3][1], control_poly[patch][i + 1][3][2]);
			m.translation(p1);
			manip->initial_mat(m);
			manip->visible(1);
			manip->callback(callBack, this);
			control_poly_line->push(p1, p2);
			act_line->push(p1, p2);
			num_lines++;
			if (startedby == man_start) {
				manip->child(control_poly_line);
				rootg()->add(manip);
			}
		}
	}
	if (startedby == draw) {
		act_line->color(GsColor::darkblue);
		rootg()->add(act_line);
	}
	if (startedby == man_comp) {
		buildTeapot(0, resolu);
	}
	t2 = gs_time();
	if (startedby == draw) {
		gsout << "Time taken to render cage is with draw:" << t2 - t1 << gsnl;
		gsout << "Number of lines used is:" << num_lines << gsnl;
	}
	else {
		gsout << "Time taken to render cage with manipulators is:" << t2 - t1 << gsnl;
	}
}

void MyViewer::buildTeapot(bool normflag, int resol) {
	double  t1, t2, norm_t1, norm_t2, norm_act_time;
	norm_act_time = 0;
	t1 = gs_time();
	int num_tri =0;
	rootg()->remove_all();
	int ind = 0;
	int res;
	res = resol - 1;
	for (int patch = 0; patch < 32; patch++) {
		int tpind = 0;
		SnModel* _model;
		_model = new SnModel;
		GsModel* m = _model->model();
		m->V.size(resol*resol);
		float x, y, z, normx, normy, normz;
		float mult = 1.0;
		float divi = float(res);
		for (int i = 0; i <= res; i++) {
			for (int j = 0; j <= res; j++) {
				x = y = z = 0.0;
				normx = normy = normz = 0.0;
				float u = mult*i / divi;
				float v = mult*j / divi;
				for (int k = 0; k <= 3; k++) {
					for (int l = 0; l <= 3; l++) {
						x += bn(u,k) * bn(v,l) * control_poly[patch][k][l][0];
						y += bn(u, k) * bn(v, l) * control_poly[patch][k][l][1];
						z += bn(u, k) * bn(v, l) * control_poly[patch][k][l][2];
					}
				}
				m->V[tpind++].set(x, y, z);
			}
		}
		for (int i = 0; i < (tpind - (res+2)); i++) {
			m->F.push().set(i, i + 1, i + res+2);
			m->F.push().set(i, i + res+2, i + res+1);
			num_tri = num_tri + 2;
		}
		
		if (normflag) {
			norm_t1 = gs_time();
			makeNormals(m, resol);
			norm_t2 = gs_time();
			norm_act_time += (norm_t2 - norm_t1);
		}
		rootg()->add(_model);
		redraw();
	}
	t2 = gs_time();
	gsout << "Time taken to render teapot with resolution [" << resol << "] is: " << t2 - t1 << gsnl;
	gsout << "Number of triangles used is: " << num_tri << gsnl;
	if (normflag) {
		gsout << "Time taken to render normals with resolution [" << resol << "] is:" << norm_act_time << gsnl;
	}
	return;
}

float MyViewer::bn(float t, int i) {
	switch (i) {
		case 0: 
			return((1 - t) * (1 - t) * (1 - t));
		case 1: 
			return(3 * t * (1 - t) * (1 - t));
		case 2: 
			return(3 * t * t * (1 - t));
		case 3: 
			return(t * t * t);
		default:
			return(99999);
	}
}

void MyViewer::makeNormals(GsModel *m, int resol) {	
		SnLines *line;
		float u1[3],u2[3];
		float v1[3],v2[3];
		line = new SnLines;
		for (int i = 0; i < resol - 1; i++) {
			for (int j = 0; j < resol - 1; j++) {

				if (j == resol - 1) {
					m->V[i*resol+j-1].get(u1);
					m->V[i*resol + j].get(u2);
					m->V[(i + 1)*resol + j].get(v1);
					m->V[i*resol + j].get(v2);
					GsVec U1(u1), U2(u2);
					GsVec V1(v1), V2(v2);
					GsVec normint;
					normint.cross((V1-V2), (U1-U2));
					normint.normalize();
					normint.len(float(0.1));
					GsVec bezPt = m->V[i*resol + j];
					GsVec norm = normint;
					line->push(bezPt, bezPt + norm);
				}
				else if (i == resol - 1) {
					m->V[i*resol + j + 1].get(u1);
					m->V[i*resol + j].get(u2);
					m->V[i*resol + j].get(v1);
					m->V[(i - 1)*resol + j].get(v2);
					GsVec U1(u1), U2(u2);
					GsVec V1(v1), V2(v2);
					GsVec normint;
					normint.cross((V2-V1), (U1-U2));
					normint.normalize();
					GsVec bezPt = m->V[i*resol + j];
					GsVec norm = normint;
					line->push(bezPt, bezPt + norm);
				}
				else {
					m->V[i*resol + j + 1].get(u1);
					m->V[i*resol + j].get(u2);
					m->V[(i + 1)*resol + j].get(v1);
					m->V[i*resol + j].get(v2);
					GsVec U1(u1),U2(u2);
					GsVec V1(v1),V2(v2);
					GsVec normint;
					normint.cross((U1-U2), (V1-V2));
					normint.normalize();
					GsVec bezPt = m->V[i*resol + j];
					GsVec norm = normint;
					line->push(bezPt, bezPt + norm);
				}
			}
		}
		rootg()->add(line);
}