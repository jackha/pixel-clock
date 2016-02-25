use <logo.scad>;

$fn = 12;

SIDE_THICKNESS = 2;
FRONT_THICKNESS = 0.5;
BACK_THICKNESS = 1.5;
DEPTH = 40;

INNER_WIDTH = 66;
OUTER_WIDTH = INNER_WIDTH + 2*SIDE_THICKNESS;
INNER_HEIGHT = 66;
OUTER_HEIGHT = INNER_HEIGHT + 2*SIDE_THICKNESS;

ROUNDING_R = 2;

HOLDER_OUTER_R = 4;
HOLDER_INNER_R = 1.3;
HOLDER_H = 10;

module rounded_cube(x, y, z, r) {
    translate([r,r,0])
    minkowski() {
        cube([x-2*r, y-2*r, 1]);
        cylinder(r=r, h=z-1);
    }
}

module tapped_hole(h1, h2, r1, r2) {
    cylinder(r=r1, h=h1);
    translate([0,0,h1-0.1])
    cylinder(r1=r1, r2=r2, h=h2);
    translate([0,0,h1+h2-0.1])
    cylinder(r=r2, h=10);
}

module base() {
union() {
difference() {
    rounded_cube(OUTER_WIDTH, OUTER_HEIGHT, DEPTH, ROUNDING_R);
    
    translate([SIDE_THICKNESS, SIDE_THICKNESS, FRONT_THICKNESS])
    cube([INNER_WIDTH, INNER_HEIGHT, DEPTH]);
    
    translate([OUTER_WIDTH/2-20, 60, 10])
    cube([40, 50, DEPTH]);

    translate([OUTER_WIDTH - 0.5, OUTER_HEIGHT/2-10, 33])
    rotate([0,90,0])
    logo(5, 20);
}

    translate([2-0.5, OUTER_HEIGHT/2-10, 8])
    rotate([0,-90,0])
    logo(2, 20);

    difference() {
        hull() {
            translate([SIDE_THICKNESS-0.1, SIDE_THICKNESS-0.1, 25])
            cylinder(r=0.1, h=1);
            translate([HOLDER_OUTER_R, HOLDER_OUTER_R, DEPTH-HOLDER_H-0.5])
            cylinder(r=HOLDER_OUTER_R, h=HOLDER_H);
        }
        translate([HOLDER_OUTER_R, HOLDER_OUTER_R, DEPTH-HOLDER_H])
        cylinder(r=HOLDER_INNER_R, h=100);
    }
    difference() {
        hull() {
            translate([OUTER_WIDTH-SIDE_THICKNESS+0.1, SIDE_THICKNESS-0.1, 25])
            cylinder(r=0.1, h=1);
            translate([OUTER_WIDTH-HOLDER_OUTER_R, HOLDER_OUTER_R, DEPTH-HOLDER_H-0.5])
            cylinder(r=HOLDER_OUTER_R, h=HOLDER_H);
        }
        translate([OUTER_WIDTH-HOLDER_OUTER_R, HOLDER_OUTER_R, DEPTH-HOLDER_H])
        cylinder(r=HOLDER_INNER_R, h=100);
    }
    difference() {
        hull() {
            translate([SIDE_THICKNESS-0.1, OUTER_HEIGHT-SIDE_THICKNESS+0.1, 25])
            cylinder(r=0.1, h=1);
            translate([HOLDER_OUTER_R, OUTER_HEIGHT-HOLDER_OUTER_R, DEPTH-HOLDER_H-0.5])
            cylinder(r=HOLDER_OUTER_R, h=HOLDER_H);
        }
        translate([HOLDER_OUTER_R, OUTER_HEIGHT-HOLDER_OUTER_R, DEPTH-HOLDER_H])
        cylinder(r=HOLDER_INNER_R, h=100);
    }
    difference() {
        hull() {
            translate([OUTER_HEIGHT-SIDE_THICKNESS+0.1, OUTER_HEIGHT-SIDE_THICKNESS+0.1, 25])
            cylinder(r=0.1, h=1);
            translate([OUTER_WIDTH-HOLDER_OUTER_R, OUTER_HEIGHT-HOLDER_OUTER_R, DEPTH-HOLDER_H-0.5])
            cylinder(r=HOLDER_OUTER_R, h=HOLDER_H);
        }
        translate([OUTER_WIDTH-HOLDER_OUTER_R, OUTER_HEIGHT-HOLDER_OUTER_R, DEPTH-HOLDER_H])
        cylinder(r=HOLDER_INNER_R, h=100);
    }
    
}
}

SCREW_R1 = 1.5;
SCREW_R2 = 3;

module back() {
   difference() {
        rounded_cube(OUTER_WIDTH, OUTER_HEIGHT, BACK_THICKNESS, ROUNDING_R);
       
       translate([OUTER_WIDTH/2-20, -1, -1])
       cube([40, 11, 10]);
            
       translate([HOLDER_OUTER_R, HOLDER_OUTER_R, -1])
       tapped_hole(r1=SCREW_R1, r2=SCREW_R2, h1=1.2, h2=1.3);

       translate([HOLDER_OUTER_R, OUTER_HEIGHT-HOLDER_OUTER_R, -1])
       tapped_hole(r1=SCREW_R1, r2=SCREW_R2, h1=1.2, h2=1.3);

       translate([OUTER_WIDTH-HOLDER_OUTER_R, HOLDER_OUTER_R, -1])
       tapped_hole(r1=SCREW_R1, r2=SCREW_R2, h1=1.2, h2=1.3);

       translate([OUTER_WIDTH-HOLDER_OUTER_R, OUTER_HEIGHT-HOLDER_OUTER_R, -1])
       tapped_hole(r1=SCREW_R1, r2=SCREW_R2, h1=1.2, h2=1.3);

   }
}

//base();
back();
