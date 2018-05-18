/*========== my_main.c ==========

  This is the only file you need to modify in order
  to get a working mdl project (for now).

  my_main.c will serve as the interpreter for mdl.
  When an mdl script goes through a lexer and parser,
  the resulting operations will be in the array op[].

  Your job is to go through each entry in op and perform
  the required action from the list below:

  push: push a new origin matrix onto the origin stack

  pop: remove the top matrix on the origin stack

  move/scale/rotate: create a transformation matrix
                     based on the provided values, then
                     multiply the current top of the
                     origins stack by it.

  box/sphere/torus: create a solid object based on the
                    provided values. Store that in a
                    temporary matrix, multiply it by the
                    current top of the origins stack, then
                    call draw_polygons.

  line: create a line based on the provided values. Store
        that in a temporary matrix, multiply it by the
        current top of the origins stack, then call draw_lines.

  save: call save_extension with the provided filename

  display: view the screen
  =========================*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "parser.h"
#include "symtab.h"
#include "y.tab.h"

#include "matrix.h"
#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "stack.h"
#include "gmath.h"

void my_main() {
  
  int i;
  struct matrix *tmp;
  struct stack *systems;
  screen t;
  zbuffer zb;
  color g;
  double step_3d = 20;
  double theta, r0, r1, axis;
  double x0, y0, z0, x1, y1, z1;

  //Lighting values here for easy access
  color ambient;
  double light[2][3];
  double view[3];
  double areflect[3];
  double dreflect[3];
  double sreflect[3];

  ambient.red = 50;
  ambient.green = 50;
  ambient.blue = 50;

  light[LOCATION][0] = 0.5;
  light[LOCATION][1] = 0.75;
  light[LOCATION][2] = 1;

  light[COLOR][RED] = 255;
  light[COLOR][GREEN] = 255;
  light[COLOR][BLUE] = 255;

  view[0] = 0;
  view[1] = 0;
  view[2] = 1;

  areflect[RED] = 0.1;
  areflect[GREEN] = 0.1;
  areflect[BLUE] = 0.1;

  dreflect[RED] = 0.5;
  dreflect[GREEN] = 0.5;
  dreflect[BLUE] = 0.5;

  sreflect[RED] = 0.5;
  sreflect[GREEN] = 0.5;
  sreflect[BLUE] = 0.5;

  systems = new_stack();
  tmp = new_matrix(4, 1000);
  clear_screen( t );
  clear_zbuffer(zb);
  g.red = 0;
  g.green = 0;
  g.blue = 0;

  for ( i = 0 ; i < lastop ; i++ ) {
    if ( op[i].opcode == PUSH ) {
      push(systems);
    }
    else if ( op[i].opcode == POP ) {
      pop(systems);
    }
    else if ( op[i].opcode == DISPLAY ) {
      display(t);
    }
    else if ( op[i].opcode == SAVE ) {
      save_extension(t, op[i].op.save.p->name);
    }
    else if ( op[i].opcode == SPHERE ) {
      x0 = op[i].op.sphere.d[0];
      y0 = op[i].op.sphere.d[1];
      z0 = op[i].op.sphere.d[2];
      r0 = op[i].op.sphere.r;
      add_sphere( tmp, x0, y0, z0, r0, step_3d );
      matrix_mult( peek(systems), tmp );
      draw_polygons( tmp, t, zb, view, light, ambient,
		     areflect, dreflect, sreflect );
      tmp->lastcol = 0;
    }
    else if ( op[i].opcode == TORUS ) {
      x0 = op[i].op.torus.d[0];
      y0 = op[i].op.torus.d[1];
      z0 = op[i].op.torus.d[2];
      r0 = op[i].op.torus.r0;
      r1 = op[i].op.torus.r1;
      add_torus( tmp, x0, y0, z0, r0, r1, step_3d );
      matrix_mult( peek(systems), tmp );
      draw_polygons( tmp, t, zb, view, light, ambient,
		     areflect, dreflect, sreflect );
      tmp->lastcol = 0;
    }
    else if ( op[i].opcode == BOX ) {
      x0 = op[i].op.box.d0[0];
      y0 = op[i].op.box.d0[1];
      z0 = op[i].op.box.d0[2];
      x1 = op[i].op.box.d1[0];
      y1 = op[i].op.box.d1[1];
      z1 = op[i].op.box.d1[2];
      add_box( tmp, x0, y0, z0, x1, y1, z1 );
      matrix_mult( peek(systems), tmp );
      draw_polygons( tmp, t, zb, view, light, ambient,
		     areflect, dreflect, sreflect );
      tmp->lastcol = 0;
    }
    else if ( op[i].opcode == LINE ) {
      x0 = op[i].op.line.p0[0];
      y0 = op[i].op.line.p0[1];
      z0 = op[i].op.line.p0[2];
      x1 = op[i].op.line.p1[0];
      y1 = op[i].op.line.p1[1];
      z1 = op[i].op.line.p1[2];
      add_edge( tmp, x0, y0, z0, x1, y1, z1 );
      matrix_mult( peek(systems), tmp );
      draw_lines( tmp, t, zb, g );
      tmp->lastcol = 0;
    }
    else if ( op[i].opcode == MOVE ) {
      x0 = op[i].op.move.d[0];
      y0 = op[i].op.move.d[1];
      z0 = op[i].op.move.d[2];
      tmp = make_translate( x0, y0, z0 );
      matrix_mult( peek(systems), tmp );
      copy_matrix( tmp, peek(systems) );
      tmp->lastcol = 0;
    }
    else if ( op[i].opcode == SCALE ) {
      x0 = op[i].op.scale.d[0];
      y0 = op[i].op.scale.d[1];
      z0 = op[i].op.scale.d[2];
      tmp = make_scale( x0, y0, z0 );
      matrix_mult( peek(systems), tmp );
      copy_matrix( tmp, peek(systems) );
      tmp->lastcol = 0;
    }
    else if ( op[i].opcode == ROTATE ) {
      theta = op[i].op.rotate.degrees;
      axis = op[i].op.rotate.axis;
      theta *= M_PI / 180;
      if ( axis == 0 ) tmp = make_rotX( theta );
      else if ( axis == 1 ) tmp = make_rotY( theta );
      else tmp = make_rotZ( theta );
      matrix_mult( peek(systems), tmp );
      copy_matrix( tmp, peek(systems) );
      tmp->lastcol = 0;
    }
  }

}
