#include <stdio.h>
#include <stdlib.h>

#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "matrix.h"
#include "math.h"
#include "gmath.h"

/*======== void scanline_convert() ==========
  Inputs: struct matrix *points
          int i
          screen s
          zbuffer zb
  Returns:

  Fills in polygon i by drawing consecutive horizontal (or vertical) lines.

  Color should be set differently for each polygon.
  ====================*/
void scanline_convert( struct matrix *points, int i, screen s, zbuffer zb ) {
  int p = i * 3;
  double top, bot, mid;
  if (points -> m[p][1] > points -> m[p+1][1] && points -> m[p][1] > points -> m[p+2][1]){
    top = p;
    if (points->m[p+1][1] > points->m[p+2][1]){
      bot = p+2;
      mid = p+1;
    }
    else {
      bot = p+1;
      mid = p+2;
    }
  }
  else if (points -> m[p+1][1] > points -> m[p][1] && points -> m[p+1][1] > points-> m[p+2][1]){
    top = p+1;
    if (points->m[p][1] > points->m[p+2][1]){
      bot = p+2;
      mid = p;
    }
    else {
      bot = p;
      mid = p+2;
    }
  }
  else {
    top = p+2;
    if (points->m[p][1] > points->m[p+1][1]){
      bot = p+1;
      mid = p;
    }
    else {
      bot = p;
      mid = p+1;
    }
  }
  double y, x0, x1, d1, d2;
  double m;
  x0 = points->m[bot][0];
  d1 = (points->m[top][0] - points->m[bot][0])/(points->m[top][1] - points->m[bot][1]);
  d2 = (points->m[mid][0] - points->m[bot][0])/(points->m[mid][1] - points->m[bot][1]);
  x1 = x0;
  color c;
  c.red = 256 % i;
  c.blue = i%255;
  c.green = (255 + i)%255;
  
  for (y = points->m[bot][1]; y <= point->m[top][1]; y++){
    draw_line(x0,y,0,x1,y,0,s,zb,c);
    if (y == points->m[mid][1]){ d2 = (points->m[top][0] - points->m[mid][0])/(points->m[top][1] - points->m[bot][1]); }
    x0 += d1;
    x1 += d2;
  }
}

/*======== void add_polygon() ==========
  Inputs:   struct matrix *surfaces
  double x0
  double y0
  double z0
  double x1
  double y1
  double z1
  double x2
  double y2
  double z2
  Returns:
  Adds the vertices (x0, y0, z0), (x1, y1, z1)
  and (x2, y2, z2) to the polygon matrix. They
  define a single triangle surface.
  ====================*/
void add_polygon( struct matrix *polygons,
                  double x0, double y0, double z0,
                  double x1, double y1, double z1,
                  double x2, double y2, double z2 ) {

  add_point(polygons, x0, y0, z0);
  add_point(polygons, x1, y1, z1);
  add_point(polygons, x2, y2, z2);
}

/*======== void draw_polygons() ==========
  Inputs:   struct matrix *polygons
  screen s
  color c
  Returns:
  Goes through polygons 3 points at a time, drawing
  lines connecting each points to create bounding
  triangles
  ====================*/
void draw_polygons(struct matrix *polygons, screen s, zbuffer zb, color c ) {
  if ( polygons->lastcol < 3 ) {
    printf("Need at least 3 points to draw a polygon!\n");
    return;
  }

  int point;
  double *normal;

  for (point=0; point < polygons->lastcol-2; point+=3) {

    normal = calculate_normal(polygons, point);

    if ( normal[2] > 0 ) {
      scanline_convert(polygons, point, s, zb);
    }
  }
}

/*======== void add_box() ==========
  Inputs:   struct matrix * edges
  double x
  double y
  double z
  double width
  double height
  double depth
  Returns:

  add the points for a rectagular prism whose
  upper-left corner is (x, y, z) with width,
  height and depth dimensions.
  ====================*/
void add_box( struct matrix * polygons,
              double x, double y, double z,
              double width, double height, double depth ) {

  double x1, y1, z1;
  x1 = x+width;
  y1 = y-height;
  z1 = z-depth;

  //front
  add_polygon(polygons, x, y, z, x1, y1, z, x1, y, z);
  add_polygon(polygons, x, y, z, x, y1, z, x1, y1, z);

  //back
  add_polygon(polygons, x1, y, z1, x, y1, z1, x, y, z1);
  add_polygon(polygons, x1, y, z1, x1, y1, z1, x, y1, z1);

  //right side
  add_polygon(polygons, x1, y, z, x1, y1, z1, x1, y, z1);
  add_polygon(polygons, x1, y, z, x1, y1, z, x1, y1, z1);
  //left side
  add_polygon(polygons, x, y, z1, x, y1, z, x, y, z);
  add_polygon(polygons, x, y, z1, x, y1, z1, x, y1, z);

  //top
  add_polygon(polygons, x, y, z1, x1, y, z, x1, y, z1);
  add_polygon(polygons, x, y, z1, x, y, z, x1, y, z);
  //bottom
  add_polygon(polygons, x, y1, z, x1, y1, z1, x1, y1, z);
  add_polygon(polygons, x, y1, z, x, y1, z1, x1, y1, z1);
}//end add_box

/*======== void add_sphere() ==========
  Inputs:   struct matrix * points
  double cx
  double cy
  double cz
  double r
  double step
  Returns:

  adds all the points for a sphere with center
  (cx, cy, cz) and radius r.

  should call generate_sphere to create the
  necessary points
  ====================*/
void add_sphere( struct matrix * edges,
                 double cx, double cy, double cz,
                 double r, int step ) {

  struct matrix *points = generate_sphere(cx, cy, cz, r, step);

  int p0, p1, p2, p3, lat, longt;
  int latStop, longStop, latStart, longStart;
  latStart = 0;
  latStop = step;
  longStart = 0;
  longStop = step;

  step++;
  for ( lat = latStart; lat < latStop; lat++ ) {
    for ( longt = longStart; longt < longStop; longt++ ) {

      p0 = lat * (step) + longt;
      p1 = p0+1;
      p2 = (p1+step) % (step * (step-1));
      p3 = (p0+step) % (step * (step-1));

      //printf("p0: %d\tp1: %d\tp2: %d\tp3: %d\n", p0, p1, p2, p3);
      if (longt <= step - 2)
        add_polygon( edges, points->m[0][p0],
                     points->m[1][p0],
                     points->m[2][p0],
                     points->m[0][p1],
                     points->m[1][p1],
                     points->m[2][p1],
                     points->m[0][p2],
                     points->m[1][p2],
                     points->m[2][p2]);
      if (longt > 0 )
        add_polygon( edges, points->m[0][p0],
                     points->m[1][p0],
                     points->m[2][p0],
                     points->m[0][p2],
                     points->m[1][p2],
                     points->m[2][p2],
                     points->m[0][p3],
                     points->m[1][p3],
                     points->m[2][p3]);
    }
  }
  free_matrix(points);
}

/*======== void generate_sphere() ==========
  Inputs:   double cx
  double cy
  double cz
  double r
  int step
  Returns: Generates all the points along the surface
  of a sphere with center (cx, cy, cz) and
  radius r.
  Returns a matrix of those points
  ====================*/
struct matrix * generate_sphere(double cx, double cy, double cz,
                                double r, int step ) {

  struct matrix *points = new_matrix(4, step * step);
  int circle, rotation, rot_start, rot_stop, circ_start, circ_stop;
  double x, y, z, rot, circ;

  rot_start = 0;
  rot_stop = step;
  circ_start = 0;
  circ_stop = step;

  for (rotation = rot_start; rotation < rot_stop; rotation++) {
    rot = (double)rotation / step;

    for(circle = circ_start; circle <= circ_stop; circle++){
      circ = (double)circle / step;

      x = r * cos(M_PI * circ) + cx;
      y = r * sin(M_PI * circ) *
        cos(2*M_PI * rot) + cy;
      z = r * sin(M_PI * circ) *
        sin(2*M_PI * rot) + cz;

      /* printf("rotation: %d\tcircle: %d\n", rotation, circle); */
      /* printf("rot: %lf\tcirc: %lf\n", rot, circ); */
      /* printf("sphere point: (%0.2f, %0.2f, %0.2f)\n\n", x, y, z); */
      add_point(points, x, y, z);
    }
  }

  return points;
}

/*======== void add_torus() ==========
  Inputs:   struct matrix * points
  double cx
  double cy
  double cz
  double r1
  double r2
  double step
  Returns:

  adds all the points required to make a torus
  with center (cx, cy, cz) and radii r1 and r2.

  should call generate_torus to create the
  necessary points
  ====================*/
void add_torus( struct matrix * edges,
                double cx, double cy, double cz,
                double r1, double r2, int step ) {

  struct matrix *points = generate_torus(cx, cy, cz, r1, r2, step);

  int p0, p1, p2, p3, lat, longt;
  int latStop, longStop, latStart, longStart;
  latStart = 0;
  latStop = step;
  longStart = 0;
  longStop = step;

  //printf("points: %d\n", points->lastcol);

  for ( lat = latStart; lat < latStop; lat++ ) {
    for ( longt = longStart; longt < longStop; longt++ ) {
      p0 = lat * step + longt;
      if (longt == step - 1)
        p1 = p0 - longt;
      else
        p1 = p0 + 1;
      p2 = (p1 + step) % (step * step);
      p3 = (p0 + step) % (step * step);

      //printf("p0: %d\tp1: %d\tp2: %d\tp3: %d\n", p0, p1, p2, p3);
      add_polygon( edges, points->m[0][p0],
                   points->m[1][p0],
                   points->m[2][p0],
                   points->m[0][p3],
                   points->m[1][p3],
                   points->m[2][p3],
                   points->m[0][p2],
                   points->m[1][p2],
                   points->m[2][p2]);
      add_polygon( edges, points->m[0][p0],
                   points->m[1][p0],
                   points->m[2][p0],
                   points->m[0][p2],
                   points->m[1][p2],
                   points->m[2][p2],
                   points->m[0][p1],
                   points->m[1][p1],
                   points->m[2][p1]);
    }
  }
  free_matrix(points);
}
/*======== void generate_torus() ==========
  Inputs:   struct matrix * points
  double cx
  double cy
  double cz
  double r
  int step
  Returns: Generates all the points along the surface
  of a torus with center (cx, cy, cz) and
  radii r1 and r2.
  Returns a matrix of those points
  ====================*/
struct matrix * generate_torus( double cx, double cy, double cz,
                                double r1, double r2, int step ) {

  struct matrix *points = new_matrix(4, step * step);
  int circle, rotation, rot_start, rot_stop, circ_start, circ_stop;
  double x, y, z, rot, circ;

  rot_start = 0;
  rot_stop = step;
  circ_start = 0;
  circ_stop = step;

  for (rotation = rot_start; rotation < rot_stop; rotation++) {
    rot = (double)rotation / step;

    for(circle = circ_start; circle < circ_stop; circle++){
      circ = (double)circle / step;

      x = cos(2*M_PI * rot) *
        (r1 * cos(2*M_PI * circ) + r2) + cx;
      y = r1 * sin(2*M_PI * circ) + cy;
      z = -1*sin(2*M_PI * rot) *
        (r1 * cos(2*M_PI * circ) + r2) + cz;

      //printf("rotation: %d\tcircle: %d\n", rotation, circle);
      //printf("torus point: (%0.2f, %0.2f, %0.2f)\n", x, y, z);
      add_point(points, x, y, z);
    }
  }
  return points;
}

/*======== void add_circle() ==========
  Inputs:   struct matrix * points
  double cx
  double cy
  double r
  double step
  Returns:

  Adds the circle at (cx, cy) with radius r to edges
  ====================*/
void add_circle( struct matrix * edges,
                 double cx, double cy, double cz,
                 double r, int step ) {
  double x0, y0, x1, y1, t;
  int i;
  x0 = r + cx;
  y0 = cy;

  for (i=1; i<=step; i++) {
    t = (double)i/step;
    x1 = r * cos(2 * M_PI * t) + cx;
    y1 = r * sin(2 * M_PI * t) + cy;

    add_edge(edges, x0, y0, cz, x1, y1, cz);
    x0 = x1;
    y0 = y1;
  }
}


/*======== void add_curve() ==========
  Inputs:   struct matrix *points
  double x0
  double y0
  double x1
  double y1
  double x2
  double y2
  double x3
  double y3
  double step
  int type
  Returns:

  Adds the curve bounded by the 4 points passsed as parameters
  of type specified in type (see matrix.h for curve type constants)
  to the matrix points
  ====================*/
void add_curve( struct matrix *edges,
                double x0, double y0,
                double x1, double y1,
                double x2, double y2,
                double x3, double y3,
                int step, int type ) {

  double t, x, y;
  struct matrix *xcoefs;
  struct matrix *ycoefs;
  int i;

  xcoefs = generate_curve_coefs(x0, x1, x2, x3, type);
  ycoefs = generate_curve_coefs(y0, y1, y2, y3, type);

  /* print_matrix(xcoefs); */
  /* printf("\n"); */
  /* print_matrix(ycoefs); */

  for (i=1; i<=step; i++) {

    t = (double)i/step;
    x = xcoefs->m[0][0] *t*t*t + xcoefs->m[1][0] *t*t+
      xcoefs->m[2][0] *t + xcoefs->m[3][0];
    y = ycoefs->m[0][0] *t*t*t + ycoefs->m[1][0] *t*t+
      ycoefs->m[2][0] *t + ycoefs->m[3][0];

    add_edge(edges, x0, y0, 0, x, y, 0);
    x0 = x;
    y0 = y;
  }

  free_matrix(xcoefs);
  free_matrix(ycoefs);
}


/*======== void add_point() ==========
  Inputs:   struct matrix * points
  int x
  int y
  int z
  Returns:
  adds point (x, y, z) to points and increment points.lastcol
  if points is full, should call grow on points
  ====================*/
void add_point( struct matrix * points, double x, double y, double z) {

  if ( points->lastcol == points->cols )
    grow_matrix( points, points->lastcol + 100 );

  points->m[0][ points->lastcol ] = x;
  points->m[1][ points->lastcol ] = y;
  points->m[2][ points->lastcol ] = z;
  points->m[3][ points->lastcol ] = 1;
  points->lastcol++;
} //end add_point

/*======== void add_edge() ==========
  Inputs:   struct matrix * points
  int x0, int y0, int z0, int x1, int y1, int z1
  Returns:
  add the line connecting (x0, y0, z0) to (x1, y1, z1) to points
  should use add_point
  ====================*/
void add_edge( struct matrix * points,
               double x0, double y0, double z0,
               double x1, double y1, double z1) {
  add_point( points, x0, y0, z0 );
  add_point( points, x1, y1, z1 );
}

/*======== void draw_lines() ==========
  Inputs:   struct matrix * points
  screen s
  color c
  Returns:
  Go through points 2 at a time and call draw_line to add that line
  to the screen
  ====================*/
void draw_lines( struct matrix * points, screen s, zbuffer zb, color c) {

  if ( points->lastcol < 2 ) {
    printf("Need at least 2 points to draw a line!\n");
    return;
  }
  int point;
  for (point=0; point < points->lastcol-1; point+=2)
    draw_line( points->m[0][point],
               points->m[1][point],
               points->m[2][point],
               points->m[0][point+1],
               points->m[1][point+1],
               points->m[2][point+1],
               s, zb, c);
}// end draw_lines




void draw_line(int x0, int y0, double z0,
               int x1, int y1, double z1,
               screen s, zbuffer zb, color c) {


  int x, y, d, A, B;
  int dy_east, dy_northeast, dx_east, dx_northeast, d_east, d_northeast;
  int loop_start, loop_end;

  //swap points if going right -> left
  int xt, yt;
  if (x0 > x1) {
    xt = x0;
    yt = y0;
    x0 = x1;
    y0 = y1;
    z0 = z1;
    x1 = xt;
    y1 = yt;
  }

  x = x0;
  y = y0;
  A = 2 * (y1 - y0);
  B = -2 * (x1 - x0);
  int wide = 0;
  int tall = 0;
  //octants 1 and 8
  if ( abs(x1 - x0) >= abs(y1 - y0) ) { //octant 1/8
    wide = 1;
    loop_start = x;
    loop_end = x1;
    dx_east = dx_northeast = 1;
    dy_east = 0;
    d_east = A;
    if ( A > 0 ) { //octant 1
      d = A + B/2;
      dy_northeast = 1;
      d_northeast = A + B;
    }
    else { //octant 8
      d = A - B/2;
      dy_northeast = -1;
      d_northeast = A - B;
    }
  }//end octant 1/8
  else { //octant 2/7
    tall = 1;
    dx_east = 0;
    dx_northeast = 1;
    if ( A > 0 ) {     //octant 2
      d = A/2 + B;
      dy_east = dy_northeast = 1;
      d_northeast = A + B;
      d_east = B;
      loop_start = y;
      loop_end = y1;
    }
    else {     //octant 7
      d = A/2 - B;
      dy_east = dy_northeast = -1;
      d_northeast = A - B;
      d_east = -1 * B;
      loop_start = y1;
      loop_end = y;
    }
  }

  while ( loop_start < loop_end ) {

    plot( s, zb, c, x, y, 0);
    if ( (wide && ((A > 0 && d > 0) ||
                   (A < 0 && d < 0)))
         ||
         (tall && ((A > 0 && d < 0 ) ||
                   (A < 0 && d > 0) ))) {
      y+= dy_northeast;
      d+= d_northeast;
      x+= dx_northeast;
    }
    else {
      x+= dx_east;
      y+= dy_east;
      d+= d_east;
    }
    loop_start++;
  } //end drawing loop
  plot( s, zb, c, x1, y1, 0 );
} //end draw_line
