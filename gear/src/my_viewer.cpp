
# include "my_viewer.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>

# include <sigogl/ws_run.h>

MyViewer::MyViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	_nbut=0;
	_animating=false;
	add_ui ();
	build_scene ();
}

void MyViewer::add_ui ()
{
	UiPanel *p, *sp;
	UiManager* uim = WsWindow::uim();
	p = uim->add_panel ( "", UiPanel::HorizLeft );
	p->add ( new UiButton ( "View", sp=new UiPanel() ) );
	{	UiPanel* p=sp;
		p->add ( _nbut=new UiCheckButton ( "Normals", EvNormals ) ); 
	}
	p->add ( new UiButton ( "Animate", EvAnimate ) );
	p->add ( new UiButton ( "Exit", EvExit ) ); p->top()->separate();
}

void MyViewer::add_model ( SnShape* s, GsVec p )
{
	SnManipulator* manip = new SnManipulator;
	GsMat m;
	m.translation ( p );
	manip->initial_mat ( m );

	SnGroup* g = new SnGroup;
	SnLines* l = new SnLines;
	l->color(GsColor::orange);
	g->add(s);
	g->add(l);
	manip->child(g);

	rootg()->add(manip);
}

void MyViewer::build_scene()
{
	SnGroup *g1, *g3,*g2,*g4,*g5;

	// SnGroup to hold the gear with sharp teeth
	g1 = new SnGroup;
	g1->separator(true);
	g1->add(_t1 = new SnTransform);
	_t1->get().translation(0, 0, 0);
	rootg()->add(g1);

	// SnGroup to hold the gear with trapezium teeth
	g2 = new SnGroup;
	g2->separator(true);
	g2->add(_t2 = new SnTransform); 
	_t2->get().translation(4.8f, 0.15f, 0);
	rootg()->add(g2);

	GsPnt c = GsPnt(0.0,0.0,0.0);
	GsPnt c1 = GsPnt(0.0, 0.0, 0.0);

	// Add the two gears with trapezium and sharp teeth to the scene
	double st1, st2, tt1, tt2, sqt1, sqt2;
	st1 = gs_time();
	make_sharp_teeth_gear(g1,c, 0.5, 1.0, 0.5,GsColor::darkgray);
	st2 = gs_time();
	tt1 = gs_time();
	make_traz_teeth_gear(g2,c1, 0.75, 1.0, 0.5,GsColor::blue);
	tt2 = gs_time();
	
	// Add another gear with sharp teeth
	g5 = new SnGroup;
	g5->separator(true);
	g5->add(_t5 = new SnTransform);
	_t5->get().translation(-2.35f, 0.9f, 0);
	make_sharp_teeth_gear(g5, c, 0.5, 1.0, 0.5,GsColor::magenta);
	rootg()->add(g5);

	// Add a gear belt to hold and coordinate the movement of the two different gears.
	g3 = new SnGroup;
	g3->separator(true);
	g3->add(_t3 = new SnTransform);
	_t3->get().translation(-2.45f, -1.75f, 0);
	rootg()->add(g3);
	sqt1 = gs_time();
	make_square_gear(g3);
	sqt2 = gs_time();
	// Add another gear with trapezium teeth
	g4 = new SnGroup;
	g4->separator(true);
	g4->add(_t4 = new SnTransform);
	_t4->get().translation(6.5f, 1.75f, 0);
	make_traz_teeth_gear(g4,c1, 0.75, 1.0, 0.5,GsColor::orange);
	rootg()->add(g4);

	gsout << "Time for traz render:" << tt2 - tt1 << gsnl;
	gsout << "Time for square gear render:" << sqt2 - sqt1 << gsnl;
	gsout << "Time for sharp gear render:" << st2 - st1;
}

// Method to create a gear with trapezium teeth
void MyViewer::make_traz_teeth_gear(SnGroup *g1,GsPnt c, double tw, double radius, double tn, GsColor color) {

	int count_faces = 0;
	double init_point[3] = { radius,0.0,0.0 };
	double init_teeth1[3] = { radius + 0.3,0.0,0.0 };
	double init_teeth2[3] = { radius + 0.3,0.0,0.0 };
	double trans_a_t1[2] = { 0.0,0.0 };

	// Rotating angle of 3 deg.
	double t = 3.14 / 60;
	double trans_mat_t1[2][2] = { cos(t),-sin(t),sin(t),cos(t) };	// Rotation matrix

	// Rotate the coordinates initially to get the coord of the teeth
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			trans_a_t1[i] = trans_a_t1[i] + (init_teeth1[j]) * trans_mat_t1[j][i];
		}
	}

	double trans_a_t2[2] = { 0.0,0.0 };

	// Rotating angle of 15 deg.
	t = 3.14 / 15;
	double trans_mat_t2[2][2] = { cos(t),-sin(t),sin(t),cos(t) };

	// Rotate the coordinates initially to get the coord of the teeth
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			trans_a_t2[i] = trans_a_t2[i] + (init_teeth2[j]) * trans_mat_t2[j][i];
		}
	}

	// Save the teeth coordinates for futuer use
	for (int i = 0; i < 2; i++) {

		init_teeth1[i] = trans_a_t1[i];
		init_teeth2[i] = trans_a_t2[i];
	}

	double x, y, z;
	x = 1.0; y = 2.0; z = 0.0;
	for (int i = 0; i < 24; i++) {
		SnModel *_model;
		_model = new SnModel;

		double trans_a[2] = { 0.0,0.0 };
		double trans_a_t1[2] = { 0.0,0.0 };
		double trans_a_t2[2] = { 0.0,0.0 };

		// Rotating angle of 15 deg
		double t = 3.14 / 12;
		double trans_mat[2][2] = { cos(t),-sin(t),sin(t),cos(t) };
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 2; j++) {
				trans_a[i] = trans_a[i] + init_point[j] * trans_mat[j][i];

			}
		}


		// Faces to add the circular body of the gear
		GsModel* m = _model->model();
		m->V.size(10);
		m->V[0].set(c.x, c.y, c.z);
		m->V[1].set(c.x + init_point[0], c.y + init_point[1], c.z + init_point[2]);
		m->V[2].set((double)c.x + trans_a[0], (double)c.y + trans_a[1], (double)c.z);
		m->V[3].set((double)c.x + init_teeth1[0], (double)c.y + init_teeth1[1], (double)c.z);
		m->V[4].set((double)c.x + init_teeth2[0], (double)c.y + init_teeth2[1], (double)c.z);
		m->V[5].set((double)c.x, (double)c.y, (double)c.z + tn);
		m->V[6].set(c.x + init_point[0], c.y + init_point[1], c.z + tn + init_point[2]);
		m->V[7].set(c.x + trans_a[0], c.y + trans_a[1], c.z + tn);
		m->V[8].set(c.x + init_teeth1[0], c.y + init_teeth1[1], c.z + tn);
		m->V[9].set(c.x + init_teeth2[0], c.y + init_teeth2[1], c.z + tn);


		m->F.push().set(0, 1, 2);
		m->F.push().set(0, 2, 1);
		m->F.push().set(0, 1, 2);
		m->F.push().set(0, 2, 1);

		m->F.push().set(5, 6, 7);
		m->F.push().set(5, 7, 6);
		m->F.push().set(5, 6, 7);
		m->F.push().set(5, 7, 6);

		m->F.push().set(1, 6, 2);
		m->F.push().set(2, 6, 7);

		count_faces += 10;
		// Faces to add the trapezium teeth of the gear.
		if (i % 2) {
			m->F.push().set(1, 3, 2);
			m->F.push().set(1, 2, 3);
			m->F.push().set(2, 4, 3);
			m->F.push().set(2, 3, 4);
			m->F.push().set(7, 8, 6);
			m->F.push().set(7, 9, 8);
			m->F.push().set(1, 6, 3);
			m->F.push().set(1, 3, 6);
			m->F.push().set(3, 6, 8);
			m->F.push().set(3, 8, 6);
			m->F.push().set(3, 8, 4);
			m->F.push().set(3, 4, 8);
			m->F.push().set(8, 9, 4);
			m->F.push().set(8, 4, 9);
			m->F.push().set(4, 9, 2);
			m->F.push().set(4, 2, 9);
			m->F.push().set(2, 9, 7);
			m->F.push().set(2, 7, 9);
			count_faces += 18;
		}
		GsMaterial mtl;
		mtl.diffuse = color;
		m->set_one_material(mtl);
		g1->add(_model);

		// Transform the matrix
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 2; j++) {

				trans_a_t1[i] = trans_a_t1[i] + (init_teeth1[j]) * trans_mat[j][i];
				trans_a_t2[i] = trans_a_t2[i] + (init_teeth2[j]) * trans_mat[j][i];
			}
		}

		for (int i = 0; i < 2; i++) {
			init_point[i] = trans_a[i];
			init_teeth1[i] = trans_a_t1[i];
			init_teeth2[i] = trans_a_t2[i];
		}

	}
	gsout << "No of faces - traz:" << count_faces << gsnl;
}


// Method to add the gear with sharp teeth
void MyViewer::make_sharp_teeth_gear(SnGroup *g1,GsPnt c, double tw, double radius, double tn, GsColor color) {
	int count_faces = 0;
	for (int i = 1; i <= 48; i++) {

	
	SnModel *_model;
	_model = new SnModel;

	GsModel* m = _model->model();

	double pi = 3.14;
	
	// Add the main circular body of the gear
	m->V.size(6);
	m->V[0].set(c.x, c.y, c.z);
	m->V[1].set((double)c.x+radius*cos((i-1)*(pi / 12)), (double)c.y+radius*sin((i-1)*(pi / 12)), (double)c.z);
	m->V[2].set((double)c.x+radius*cos(i*(pi / 12)), (double)c.y+radius*sin(i*(pi / 12)), (double)c.z);
	m->V[3].set((double)c.x, (double)c.y, (double)c.z+tn);
	m->V[4].set(c.x+radius*cos(i*(pi / 12)), c.y+radius*sin(i*(pi / 12)), c.z+tn);
	m->V[5].set(c.x+radius*cos((i - 1)*(pi / 12)), c.y+radius*sin((i - 1)*(pi / 12)), c.z+tn);

	m->F.push().set(0, 1, 2);
	m->F.push().set(0, 2, 1);
	m->F.push().set(3, 4, 5);
	m->F.push().set(3, 5, 4);
	count_faces += 4;
	GsMaterial mtl;
	mtl.diffuse = color;
	m->set_one_material(mtl);

	// Add the sharp teeth of the gear.
	if (i % 2) {
		SnModel *teeth3;
		teeth3 = new SnModel;
		GsModel* m1 = teeth3->model();
		m1->V.size(6);
		m1->V[0].set((double)c.x+radius*cos((i-1)*(pi / 12)), (double)c.y+(radius)*sin((i-1)*(pi / 12)), (double)c.z);
		m1->V[1].set(c.x+(radius+tw)*cos(i*(pi / 12)), (double)c.y+(radius+tw)*sin(i*(pi / 12)), (double)c.z);
		m1->V[2].set((double)c.x+radius*cos(i*(pi / 12)), (double)c.y+radius*sin(i*(pi / 12)), (double)c.z);
		m1->V[3].set(c.x+(radius + tw)*cos(i*(pi / 12)), c.y+(radius + tw)*sin(i*(pi / 12)), c.z+tn);
		m1->V[4].set(c.x+radius*cos((i - 1)*(pi / 12)), c.y+radius*sin((i - 1)*(pi / 12)), c.z+tn);
		m1->V[5].set(c.x+radius*cos(i*(pi / 12)), c.y+radius*sin(i*(pi / 12)), c.z+tn);
		m1->F.push().set(0, 1, 2);
		m1->F.push().set(0, 2, 1);
		m1->F.push().set(3,4,5);
		m1->F.push().set(3, 5, 4);
		m1->F.push().set(0, 1, 3);
		m1->F.push().set(0, 3, 1);
		m1->F.push().set(0, 3, 4);
		m1->F.push().set(0, 4, 3);
		count_faces += 8;
		GsMaterial mtl1;
		mtl1.diffuse = color;
		m1->set_one_material(mtl1);
		g1->add(teeth3);
	}
	else {

		SnModel *teeth4;
		teeth4 = new SnModel;
		GsModel* m2 = teeth4->model();
		m2->V.size(6);
		m2->V[0].set((double)c.x + (radius + tw)*cos((i - 1)*(pi / 12)), (double)c.y + (radius + tw)*sin((i - 1)*(pi / 12)), (double)c.z);
		m2->V[1].set((double)c.x + radius*cos(i*(pi / 12)), (double)c.y + radius*sin(i*(pi / 12)), (double)c.z);
		m2->V[2].set((double)c.x + radius*cos((i - 1)*(pi / 12)), (double)c.y + radius*sin((i - 1)*(pi / 12)), (double)c.z);
		m2->V[3].set(c.x + radius*cos((i - 1)*(pi / 12)), c.y + radius*sin((i - 1)*(pi / 12)), c.z + tn);
		m2->V[4].set(c.x + radius*cos(i*(pi / 12)), c.y + radius*sin(i*(pi / 12)), c.z + tn);
		m2->V[5].set(c.x + (radius + tw)*cos((i - 1)*(pi / 12)), c.y + (radius + tw)*sin((i - 1)*(pi / 12)), c.z + tn);
		m2->F.push().set(0, 1, 2);
		m2->F.push().set(0, 2, 1);
		m2->F.push().set(3, 4, 5);
		m2->F.push().set(3, 5, 4);
		m2->F.push().set(1, 4, 5);
		m2->F.push().set(0, 5, 4);
		m2->F.push().set(0, 1, 5);
		m2->F.push().set(0, 5, 1);
		count_faces += 8;
		GsMaterial mtl2;
		mtl2.diffuse = color;
		m2->set_one_material(mtl2);
		g1->add(teeth4);
	}
	g1->add(_model);
	}
	gsout << "No of faces:" << count_faces << gsnl;
}


// Method to add the gear belt
void MyViewer::make_square_gear(SnGroup *g1) {

	int count_faces = 0;
	// Add the belt with sharp grooves
	for (int i = 0; i <= 8; i++) {
		SnModel *_model;
		_model = new SnModel;
		//double b_tn = 1.0;
		double t_tw = 0.25;
		GsModel* m = _model->model();
		double pi = 3.14;
		//int n = 3;
		m->V.size(10);
		m->V[0].set(2*(i - 1)*t_tw, 0.0, 0.0);
		m->V[1].set((2 * i)*t_tw, 2 * t_tw, 0.0);
		m->V[2].set(2 *(i + 1)*t_tw, 0.0, 0.0);
		m->V[3].set(2 * (i - 1)*t_tw, 0.0, 0.5);
		m->V[4].set((2 * i)*t_tw, 2 * t_tw, 0.5);
		m->V[5].set(2 * (i + 1)*t_tw, 0.0, 0.5);
		m->V[6].set(2 * (i - 1)*t_tw, -0.5, 0.0);
		m->V[7].set(2 * (i + 1)*t_tw, -0.5, 0.0);
		m->V[8].set(2 * (i - 1)*t_tw, -0.5, 0.5);
		m->V[9].set(2 * (i + 1)*t_tw, -0.5, 0.5);
		if(i%2){
		m->F.push().set(0, 2, 1);
		m->F.push().set(0, 1, 2);
		m->F.push().set(3, 5, 4);
		m->F.push().set(3, 4, 5);
		m->F.push().set(0, 2, 5);
		m->F.push().set(0, 5, 2);
		m->F.push().set(0, 5, 3);
		m->F.push().set(0, 3, 5);
		m->F.push().set(4, 2, 5);
		m->F.push().set(4, 5, 2);
		m->F.push().set(1, 4, 2);
		m->F.push().set(1, 2, 4);
		m->F.push().set(0, 4, 1);
		m->F.push().set(0, 1, 4);
		m->F.push().set(0, 4, 3);
		m->F.push().set(0, 3, 4);
		count_faces += 16;


		GsMaterial mtl;
		mtl.diffuse = GsColor::black;
		m->set_one_material(mtl);
		g1->add(_model);
		}
		m->F.push().set(6, 3, 0);
		m->F.push().set(6, 0, 3);
		m->F.push().set(6, 2, 0);
		m->F.push().set(6, 0, 2);
		m->F.push().set(6, 8, 3);
		m->F.push().set(6, 3, 8);
		m->F.push().set(8, 9, 5);
		m->F.push().set(8, 5, 9);
		m->F.push().set(8, 5, 3);
		m->F.push().set(8, 3, 5);
		m->F.push().set(6, 7, 2);
		m->F.push().set(6, 2, 7);
		m->F.push().set(6, 8, 7);
		m->F.push().set(6, 7, 8);
		m->F.push().set(8, 9, 7);
		m->F.push().set(8, 7, 9);
		m->F.push().set(7, 9, 5);
		m->F.push().set(7, 5, 9);
		m->F.push().set(7, 5, 2);
		m->F.push().set(7, 2, 5);
		count_faces += 20;
		GsMaterial mtl1;
		mtl1.diffuse = GsColor::black;
		m->set_one_material(mtl1);
		g1->add(_model);
	}

	// Add the belt with trapezium grooves
	for (int i = 60; i <= 100; i++) {
		SnModel *_model;
		_model = new SnModel;
		//double b_tn = 1.0;
		double t_tw = 0.1;
		GsModel* m = _model->model();
		m->V.size(16);
		m->V[0].set((i-1)*t_tw,0.0,0.0);
		m->V[1].set(i*t_tw,0.0,0.0);
		m->V[2].set(i*t_tw,0.75,0.0);
		m->V[3].set((i+1)*t_tw,0.0,0.0);
		m->V[4].set((i + 1)*t_tw, 0.75, 0.0);
		m->V[5].set((i + 2)*t_tw, 0.0, 0.0);

		m->V[6].set((i - 1)*t_tw, 0.0, 0.5);
		m->V[7].set(i*t_tw, 0.0, 0.5);
		m->V[8].set(i*t_tw, 0.75, 0.5);
		m->V[9].set((i + 1)*t_tw, 0.0, 0.5);
		m->V[10].set((i + 1)*t_tw, 0.75, 0.5);
		m->V[11].set((i + 2)*t_tw, 0.0, 0.5);
		m->V[12].set((i - 1)*t_tw, -0.5, 0.0);
		m->V[13].set((i + 2)*t_tw, -0.5, 0.0);
		m->V[14].set((i - 1)*t_tw, -0.5, 0.5);
		m->V[15].set((i + 2)*t_tw, -0.5, 0.5);
		if (!(i % 9)) {
			m->F.push().set(0, 1, 2);
			m->F.push().set(0, 2, 1);
			m->F.push().set(1, 3, 4);
			m->F.push().set(1, 4, 2);
			m->F.push().set(3, 5, 4);
			
			m->F.push().set(6, 7, 8);
			m->F.push().set(6, 8, 7);
			m->F.push().set(7, 9, 10);
			m->F.push().set(7, 10, 8);
			m->F.push().set(9, 11, 10);

			m->F.push().set(0, 6, 2);
			m->F.push().set(0, 2, 6);
			m->F.push().set(6, 8, 2);
			m->F.push().set(6, 2, 8);
			m->F.push().set(8, 10, 4);
			m->F.push().set(8, 4, 10);
			m->F.push().set(2, 8, 4);
			m->F.push().set(2, 4, 8);
			m->F.push().set(4, 5, 11);
			m->F.push().set(4, 11, 5);
			m->F.push().set(4, 11, 10);
			m->F.push().set(4, 10, 11);
			m->F.push().set(0, 5, 4);
			m->F.push().set(0, 4, 5);
			m->F.push().set(0, 4, 2);
			m->F.push().set(0, 2, 4);
			count_faces += 26;
			GsMaterial mtl;
			mtl.diffuse = GsColor::black;
			m->set_one_material(mtl);
			g1->add(_model);
		}
		m->F.push().set(12, 6, 0);
		m->F.push().set(12, 0, 6);
		m->F.push().set(12, 14, 6);
		m->F.push().set(12, 6, 14);
		m->F.push().set(12, 14, 13);
		m->F.push().set(12, 13, 14);
		m->F.push().set(14, 15, 13);
		m->F.push().set(14, 13, 15);
		m->F.push().set(13, 11, 5);
		m->F.push().set(13, 5, 11);
		m->F.push().set(13, 15, 11);
		m->F.push().set(13, 11, 15);
		m->F.push().set(14, 15, 11);
		m->F.push().set(14, 11, 15);
		m->F.push().set(14, 11, 6);
		m->F.push().set(14, 6, 11);
		m->F.push().set(12, 13, 5);
		m->F.push().set(12, 5, 13);
		m->F.push().set(12, 5, 0);
		m->F.push().set(12, 0, 5);
		count_faces += 20;
		GsMaterial mtl1;
		mtl1.diffuse = GsColor::black;
		m->set_one_material(mtl1);
		g1->add(_model);
	}
	gsout << "No of faces - square:" << count_faces << gsnl;
	
}
// Below is an example of how to control the main loop of an animation:
void MyViewer::run_animation()
{
	if (_animating) return; // avoid recursive calls
	_animating = true;

	int ind = gs_random(0, rootg()->size() - 1); // pick one child
	SnManipulator* manip = rootg()->get<SnManipulator>(ind); // access one of the manipulators
	GsMat m = manip->mat();

	float _curang1 = 0;
	float _curang2 = 0;
	float _curang3 = 0;
	float _curang4 = 0;
	float _curdisp = 0;
	int dir = 1;
	int ud_flag = 0;

	double frdt = 1.0 / 30.0; // delta time to reach given number of frames per second
	double v = 4; // target velocity is 1 unit per second
	double t = 0, lt = 0, t0 = gs_time();
	do // run for a while:
	{
		while (t - lt<frdt) t = gs_time() - t0; // wait until it is time for next frame
		lt = t;

		// Rotate the gear on z-axis 
		GsVec tr;
		_t1->get().getrans(tr);
		_t1->get().rotz(GS_TODEG(_curang1));
		_t1->get().setrans(tr);
		_curang1 += dir*0.0005f; // delta angle along with direction

		_t2->get().getrans(tr);
		_t2->get().rotz(GS_TODEG(_curang2));
		_t2->get().setrans(tr);
		_curang2 += dir*0.0005f; // delta angle along with direction

		_t4->get().getrans(tr);
		_t4->get().rotz(GS_TODEG(_curang3));
		_t4->get().setrans(tr);
		_curang3 -= dir*0.0005f; // delta angle along with direction

		_t5->get().getrans(tr);
		_t5->get().rotz(GS_TODEG(_curang3));
		_t5->get().setrans(tr);
		_curang4 -= dir*0.0005f;

		// This is to create the pendulum action of the belt.
		_t3->get().translation(-2.45f+_curdisp, -1.75f, 0);
		_curdisp += dir*0.05f; // delta displacement along with direction

		// Change the direction of the rotation/translation every 20th delta time
		ud_flag += 1;
		if (!(ud_flag % 20)) {
			dir = -(dir);
		}

		render(); // notify it needs redraw
		ws_check(); // redraw now
	} while (true);// m.e24 > 0 );
	_animating = false;
}

void MyViewer::show_normals ( bool b )
{
	// Note that primitives are only converted to meshes in GsModel
	// at the first draw call.


	GsArray<GsVec> fn;
	SnGroup* r = (SnGroup*)root();
	for ( int k=0; k<r->size(); k++ )
	{	SnManipulator* manip = r->get<SnManipulator>(k);
		SnShape* s = manip->child<SnGroup>()->get<SnShape>(0);
		SnLines* l = manip->child<SnGroup>()->get<SnLines>(1);
		if ( !b ) { l->visible(false); continue; }
		l->visible ( true );
		if ( !l->empty() ) continue; // build only once
		l->init();
		if ( s->instance_name()==SnPrimitive::class_name )
		{	GsModel& m = *((SnModel*)s)->model();
			m.get_normals_per_face ( fn );
			const GsVec* n = fn.pt();
			float f = 0.33f;
			for ( int i=0; i<m.F.size(); i++ )
			{	const GsVec& a=m.V[m.F[i].a]; l->push ( a, a+(*n++)*f );
				const GsVec& b=m.V[m.F[i].b]; l->push ( b, b+(*n++)*f );
				const GsVec& c=m.V[m.F[i].c]; l->push ( c, c+(*n++)*f );
			}
		}  
	}
}

int MyViewer::handle_keyboard ( const GsEvent &e )
{
	int ret = WsViewer::handle_keyboard ( e ); // 1st let system check events
	if ( ret ) return ret;

	switch ( e.key )
	{	case GsEvent::KeyEsc : gs_exit(); return 1;
		case 'n' : { bool b=!_nbut->value(); _nbut->value(b); show_normals(b); return 1; }
		default: gsout<<"Key pressed: "<<e.key<<gsnl;
	}

	return 0;
}

int MyViewer::uievent ( int e )
{
	switch ( e )
	{	case EvNormals: show_normals(_nbut->value()); return 1;
		case EvAnimate: run_animation(); return 1;
		case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}