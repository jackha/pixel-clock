/*
Use this as an extra face plate for the Pixel Clock!
*/

use <Write.scad>

INNER_WIDTH = 70.5;  
INNER_HEIGHT = 70.5;

ROUNDING_R = 4;
ROUNDING_R_INNER = 3;

SIDE_THICKNESS = 1.5;
FRONT_THICKNESS = 1;

OUTER_WIDTH = INNER_WIDTH + 2 * SIDE_THICKNESS;
OUTER_HEIGHT = INNER_HEIGHT + 2 * SIDE_THICKNESS;

DEPTH = 15;

FONT_SIZE = 12;
FONT_SCALE_Y = 0.6;

FONT_X_OFFSET = 4;
FONT_Y_OFFSET = 5;
FONT_Y_SPACING = 8;

LETTERS = [
    "VIJFVOOR", "OVERHALF", "TWEEAALF", "NEGENEEN",
    "DRIEVIER", "VIJFZZES", "ZEVENELF", "ACHTIENZ"];

module rounded_cube(x, y, z, r) {
    translate([r,r,0])
    minkowski() {
        cube([x-2*r, y-2*r, 1]);
        cylinder(r=r, h=z-1);
    }
}

module rounded_cube2(x, y, z, r, x2, y2, r2) {
    hull() {
        translate([r,r,0])
        minkowski() {
            cube([x-2*r, y-2*r, 0.1]);
            cylinder(r=r, h=0.1);
        }

        translate([r2-(x2-x)/2,r2-(y2-y)/2,z-0.2])
        minkowski() {
            cube([x2-2*r2, y2-2*r2, 0.1]);
            cylinder(r=r2, h=0.1);
        }
    
    }
}

difference() {
    rounded_cube(
        OUTER_WIDTH, OUTER_HEIGHT, DEPTH, ROUNDING_R); 

    translate([SIDE_THICKNESS, SIDE_THICKNESS, FRONT_THICKNESS])
    rounded_cube(
        INNER_WIDTH, INNER_HEIGHT, DEPTH, ROUNDING_R_INNER); 

    *rounded_cube2(
        OUTER_WIDTH, OUTER_HEIGHT, DEPTH, ROUNDING_R, 
        INNER_WIDTH, INNER_HEIGHT, ROUNDING_R_INNER);
    
    for (y=[0:7]) {
        translate([OUTER_WIDTH-FONT_X_OFFSET,FONT_Y_OFFSET+y*FONT_Y_SPACING,-0.1])
        mirror([-1,0,0])
        scale([1, FONT_SCALE_Y, 1])
        write(LETTERS[7-y],t=5, h=FONT_SIZE, center=false, font="orbitron.dxf");
    }
}