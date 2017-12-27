
# include "my_viewer.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>

# include <sigogl/ws_run.h>

static bool calledonce;
static void changeEndEffector(SnManipulator* manip, const GsEvent& e, void* udata)
{
	MyViewer* v = (MyViewer*)udata;
	GsMat& m = manip->mat();
	float pnt[3];
	int update_ee;
	m.getrans(pnt[0], pnt[1], pnt[2]);
	GsPnt clicked_point(pnt);
	if (manip == v->manip1 ) {
		update_ee = 0;
	}	
	else {
		update_ee = 1;
	}	
	v->update(clicked_point,update_ee);	
}

MyViewer::MyViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	g = new SnGroup;
	_nbut=0;
	_animating=false;
	add_ui ();
	link_lines = new SnLines;
	originP.set(0.0, 0.0, 0.0);
	links[palm][0] = originP;
	joints[palm] = 5;
	joints[finger1] = 5;
	joints[finger2] = 5;
	
	linkLength[palm][0] = 4.0;
	totalLength[palm] = linkLength[palm][0];
	for (int i = 1; i < joints[palm]-1; i++) {
		linkLength[palm][i] = 2.0;
		totalLength[palm] += linkLength[palm][i];
	}
	gsout << "Total length of palm:" << totalLength[palm] << gsnl;

	totalLength[finger1] = 0;
	for (int i = 0; i < joints[finger1]-1; i++) {
		linkLength[finger1][i] = 2.0;
		totalLength[finger1] += linkLength[finger1][i];
	}
	gsout << "Total length of finger 1:" << totalLength[finger1] << gsnl;

	totalLength[finger2] = 0;
	for (int i = 0; i < joints[finger2]-1; i++) {
		linkLength[finger2][i] = 2.0;
		totalLength[finger2] += linkLength[finger2][i];
	}
	gsout << "Total length of finger 2:" << totalLength[finger2] << gsnl;

	for (int i = 1; i < joints[palm]; i++) {
		links[palm][i].set(links[palm][i - 1].x + linkLength[palm][i - 1], 0, 0);
	}
	
	links[finger1][0].set(links[palm][joints[palm] - 1]);
	links[finger2][0].set(links[palm][joints[palm] - 1]);
	
	for (int i = 1; i < joints[finger1]; i++) {		
		links[finger1][i].set(links[finger1][i - 1].x, links[finger1][i - 1].y + linkLength[finger1][i - 1], 0);
		links[finger2][i].set(links[finger2][i - 1].x, links[finger2][i - 1].y - linkLength[finger2][i - 1], 0);
	}

	build_scene();



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

void MyViewer::add_model ( SnShape* s, GsVec p, SnManipulator *manip )
{
	
	GsMat m;
	m.translation ( p );
	manip->initial_mat ( m );

	manip->child(s);
	manip->callback(changeEndEffector, this);
	rootg()->add(manip);
}

void MyViewer::add_fingers(GsVec bot_center, GsVec top_center,SnGroup *g) {
	SnPrimitive *p = new SnPrimitive();
	p->prim().cylinder(bot_center, top_center, (float)0.1,10);
	SnPrimitive *s = new SnPrimitive;

	GsMat m;
	m.translation(bot_center);
	SnManipulator *man = new SnManipulator;
	man->initial_mat(m);
	s->prim().sphere((float)0.2);
	man->child(s);
	g->add(man);
	g->add(p);
}


void MyViewer::build_scene ()
{
	
	for (int i = 1; i < joints[palm]; i++) {
		add_fingers(links[palm][i - 1], links[palm][i], g);
	}
	

	for (int i = 1; i < joints[finger1]; i++) {
		add_fingers(links[finger1][i - 1], links[finger1][i], g);
	}

	for (int i = 1; i < joints[finger2]; i++) {
		add_fingers(links[finger2][i - 1], links[finger2][i], g);
	}
	
	rootg()->add(g);
	
	manip1 = new SnManipulator;
	p1 = new SnPrimitive(GsPrimitive::Box, (float)0.1, (float)0.1, (float)0.1);
	p1->prim().material.diffuse = GsColor::blue;
	efr[ee1].set(links[finger1][joints[finger1] - 1]);
	add_model(p1, links[finger1][joints[finger1] - 1],manip1);
	
	manip2 = new SnManipulator;
	p2 = new SnPrimitive(GsPrimitive::Box, (float)0.1, (float)0.1, (float)0.1);
	p2->prim().material.diffuse = GsColor::blue;
	efr[ee2].set(links[finger2][joints[finger2] - 1]);
	add_model(p2, links[finger2][joints[finger2] - 1],manip2);

}


void MyViewer::draw_links(int partchanged)
{
	g->init();
	link_lines->init();
	
	for (int i = 1; i < joints[palm]; i++) {	
		add_fingers(links[palm][i - 1], links[palm][i], g);
	}


	for (int i = 1; i < joints[finger1]; i++) {	
		add_fingers(links[finger1][i - 1], links[finger1][i], g);
	}

	for (int i = 1; i < joints[finger2]; i++) {	
		add_fingers(links[finger2][i - 1], links[finger2][i], g);
	}

	if (partchanged == finger1) {
		GsMat m = manip2->mat();
		m.translation(links[finger2][joints[finger2]-1]);
		manip2->initial_mat(m);
	}
		
	else {	
		GsMat m = manip1->mat();
		m.translation(links[finger1][joints[finger1]-1]);
		manip1->initial_mat(m);
	}

	rootg()->add(link_lines);
	render();
	ws_check();
	
}

void MyViewer::adjustManipulator(int e) {
	if (e == finger2) {
		GsMat m = manip2->mat();
		m.translation(links[finger2][joints[finger2] - 1]);
		manip2->initial_mat(m);
	}

	else {
		GsMat m = manip1->mat();
		m.translation(links[finger1][joints[finger1] - 1]);
		manip1->initial_mat(m);
	}
	
}

// Below is an example of how to control the main loop of an animation:
void MyViewer::run_animation ()
{
	if ( _animating ) return; // avoid recursive calls
	_animating = true;
	GsVec targetPoint;
	float y;
	float x;
	double frdt = 1.0 / 30.0; // delta time to reach given number of frames per second
	double v = 4; // target velocity is 1 unit per second
	double t = 0, lt = 0, t0 = gs_time();
	float i = 0;
	float r = 10.0;
	do // run for a while:
	{
		while (t - lt<frdt) t = gs_time() - t0; // wait until it is time for next frame
		double yinc = (t - lt)*v;
		if (t>2) yinc = -yinc; // after 2 secs: go down
		lt = t;
		y = (float)sin(10*i);
		x = (float)cos(10*i);
		
		targetPoint.set(x, y, x*y+20);
		update(targetPoint, ee1);
		adjustManipulator(ee1);

		render(); // notify it needs redraw
		ws_check(); // redraw now
		
		i+=(float)0.01;
	} while (i<5);
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

void MyViewer::update(GsPnt targetP,int e) {
	double time_beg, time_end;
	time_beg = gs_time();
	int total_iter = 0;
	if (e == 0) {
		efr[e] = targetP;
		total_iter += fabrik_backward(targetP, finger1);
		GsVec centroid = (links[finger1][0] + links[finger2][0]) / 2;
		total_iter += fabrik(centroid,palm);
		total_iter += fabrik_forward(links[palm][joints[palm]-1],finger1);
		total_iter += fabrik_forward(links[palm][joints[palm] - 1],finger2);
		draw_links(finger1);
	}
	else {
		efr[e] = targetP;
		total_iter += fabrik_backward(targetP, finger2);
		GsVec centroid = (links[finger1][0] + links[finger2][0]) / 2;
		total_iter += fabrik(centroid,palm);
		total_iter += fabrik_forward(links[palm][joints[palm] - 1], finger1);
		total_iter += fabrik_forward(links[palm][joints[palm] - 1], finger2);
		draw_links(finger2);
	}
	time_end = gs_time();
	gsout << "Total iteration steps are: " << total_iter << gsnl;
	gsout << "Total time taken is: " << time_end - time_beg << gsnl;
	return;
}

int MyViewer::fabrik(GsPnt targetP, int part) {
	int iter = 0;
	float tolerance = (float)0.1;
	float distance = calculateDistance(links[part][0], targetP);
	
	if (distance > totalLength[part]) {
		for (int i = 0; i < joints[part]-1; i++) {
			float r = calculateDistance(links[part][i], targetP);
			float l = linkLength[part][i] / r;
			GsVec temp;
			temp = (1 - l)*links[part][i] + l*targetP;
			links[part][i + 1] = temp;
			iter++;
		}
	}
	else {
		int count = 0;
		float diff = calculateDistance(links[part][joints[part] - 1], targetP);
		while (diff > tolerance) {
			iter += fabrik_backward(targetP,part);
			iter += fabrik_forward(originP,part);
			diff = calculateDistance(links[part][joints[part] - 1], targetP);
			count++;
			calledonce = 1;
			if (count > 10) {
				gsout << "Tried for more than 10 times!" << gsnl;
				break;
			}
			gsout << "fabrik iteration steps are inside while:" << iter << gsnl;
		}	
	}
	gsout << "fabrik iteration steps are:" << iter << gsnl;
	return iter;
}

int MyViewer::fabrik_backward(GsPnt targetP,int part) {
	int iter = 0;
	links[part][joints[part] - 1] = targetP;
	
	for (int i = joints[part] - 2; i >= 0; i--) {
		float r = calculateDistance(links[part][i],links[part][i+1]);
		float l = linkLength[part][i-1] / r;
		GsVec temp;
		temp = (1 - l)*links[part][i+1] + l*links[part][i];
		links[part][i]=temp;
		iter++;
	}
	gsout << "fabrik backward iterations are: " << iter << gsnl;
	return iter;
}

int MyViewer::fabrik_forward(GsVec beginP,int part) {
	int iter = 0;
	links[part][0] = beginP;
	for (int i = 0; i < joints[part] - 1; i++) {
		float r = calculateDistance(links[part][i], links[part][i+1]);
		float l = linkLength[part][i] / r;
		GsVec temp;
		temp = (1 - l)*links[part][i] + l*links[part][i + 1];
		links[part][i+1]=temp;
		iter++;
	}
	gsout << "fabrik forward iterations are: " << iter << gsnl;
	return iter;
}

float MyViewer::calculateDistance(GsVec a, GsVec b) {
	return((float)sqrt((b.x-a.x)*(b.x - a.x)+ (b.y - a.y)*(b.y - a.y)+ (b.z - a.z)*(b.z - a.z)));
}