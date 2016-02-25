use <logo.scad>;

$fn = 20;

SIDE_THICKNESS = 2;
FRONT_THICKNESS = 0.5;
BACK_THICKNESS = 2;
DEPTH = 27;

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
    
    translate([OUTER_WIDTH - 0.7, OUTER_HEIGHT/2-7, 22])
    rotate([0,90,0])
    logo(5, 12);
}

    translate([2-0.7, OUTER_HEIGHT/2-7, 8])
    rotate([0,-90,0])
    logo(2, 12);

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
    *difference() {
        hull() {
            translate([OUTER_WIDTH-SIDE_THICKNESS+0.1, SIDE_THICKNESS-0.1, 25])
            cylinder(r=0.1, h=1);
            translate([OUTER_WIDTH-HOLDER_OUTER_R, HOLDER_OUTER_R, DEPTH-HOLDER_H-0.5])
            cylinder(r=HOLDER_OUTER_R, h=HOLDER_H);
        }
        translate([OUTER_WIDTH-HOLDER_OUTER_R, HOLDER_OUTER_R, DEPTH-HOLDER_H])
        cylinder(r=HOLDER_INNER_R, h=100);
    }
    *difference() {
        hull() {
            translate([SIDE_THICKNESS-0.1, OUTER_HEIGHT-SIDE_THICKNESS+0.1, 25])
            cylinder(r=0.1, h=1);
            translate([HOLDER_OUTER_R, OUTER_HEIGHT-HOLDER_OUTER_R, DEPTH-HOLDER_H-0.5])
            cylinder(r=HOLDER_OUTER_R, h=HOLDER_H);
        }
        translate([HOLDER_OUTER_R, OUTER_HEIGHT-HOLDER_OUTER_R, DEPTH-HOLDER_H])
        cylinder(r=HOLDER_INNER_R, h=100);
    }
    *difference() {
        hull() {
            translate([OUTER_HEIGHT-SIDE_THICKNESS+0.1, OUTER_HEIGHT-SIDE_THICKNESS+0.1, 25])
            cylinder(r=0.1, h=1);
            translate([OUTER_WIDTH-HOLDER_OUTER_R, OUTER_HEIGHT-HOLDER_OUTER_R, DEPTH-HOLDER_H-0.5])
            cylinder(r=HOLDER_OUTER_R, h=HOLDER_H);
        }
        translate([OUTER_WIDTH-HOLDER_OUTER_R, OUTER_HEIGHT-HOLDER_OUTER_R, DEPTH-HOLDER_H])
        cylinder(r=HOLDER_INNER_R, h=100);
    }


    // holder
        hull() {
            translate([OUTER_HEIGHT-SIDE_THICKNESS+0.1, OUTER_HEIGHT-SIDE_THICKNESS+0.1, DEPTH-3-2])
            cylinder(r=0.1, h=2);
            translate([OUTER_HEIGHT-SIDE_THICKNESS-1, OUTER_HEIGHT-SIDE_THICKNESS-1, DEPTH-1-2])
            cylinder(r=2, h=1);
        }

        hull() {
            translate([OUTER_HEIGHT-SIDE_THICKNESS-1, OUTER_HEIGHT-SIDE_THICKNESS-1, DEPTH-1-2])
            cylinder(r=2, h=1);
            translate([OUTER_HEIGHT-SIDE_THICKNESS+0.1, OUTER_HEIGHT-SIDE_THICKNESS+0.1, DEPTH-2])
            cylinder(r=0.1, h=2);
        }
    
}
}

SCREW_R1 = 1.5;
SCREW_R2 = 3;


MOUNT_SCREW_R = 1.5;  // for base pcb
MOUNT_SCREW_OUTER_R = 3;

MOUNT_SCREW_R2 = 1.1;  // for rtc module
MOUNT_SCREW_OUTER_R2 = 2.5;  // for rtc module

module mounting_tube(h) {
    difference() {
        cylinder(r=MOUNT_SCREW_OUTER_R, h=h);
        translate([0,0,-1])
        cylinder(r=MOUNT_SCREW_R, h=h+2);
    }
}

module pcb_mounting_tubes(h) {
    translate([0.1*25.4, 0.1*25.4, 0])
    mounting_tube(h);

    translate([2.4*25.4, 0.1*25.4, 0])
    mounting_tube(h);

    translate([0.1*25.4, 1.4*25.4, 0])
    mounting_tube(h);

    translate([2.4*25.4, 1.4*25.4, 0])
    mounting_tube(h);

    translate([(2.5-0.45) * 25.4, 1.1*25.4, 0])
    cylinder(r=7, h=2);
}

PRG_MARGIN = 1;

module pcb_holes() {
    translate([(2.5-0.45) * 25.4, 1.1*25.4, -10])
    cylinder(r=4, h=50);

    translate([(2.5-0.45) * 25.4, 1.1*25.4 + 6, 2-1])
    cylinder(r=1.2, h=2);

    translate([0.7 * 25.4-PRG_MARGIN, 0.35*25.4-PRG_MARGIN, -10])
    cube([0.6*25.4+2*PRG_MARGIN, 2.54+2*PRG_MARGIN, 15]);
}

module mounting_tube_rtc(h) {
    difference() {
        cylinder(r=MOUNT_SCREW_OUTER_R2, h=h);
        translate([0,0,-1])
        cylinder(r=MOUNT_SCREW_R2, h=h+2);
    }
}

module rtc_mounts(h) {
    translate([-4, 2, 0])
    mounting_tube_rtc(h);

    translate([-29.8, 20, 0])
    mounting_tube_rtc(h);

    translate([-4, 20, 0])
    mounting_tube_rtc(h);
}

HOOK_R = 2;

module back() {
   difference() {
       union() {
           rounded_cube(OUTER_WIDTH, OUTER_HEIGHT, BACK_THICKNESS, ROUNDING_R);
           
           translate([3,27,BACK_THICKNESS-0.1])
           pcb_mounting_tubes(9);

            // support for display
            hull() {
           translate([64,24.7,BACK_THICKNESS-0.1])
           cylinder(r=2, h=DEPTH-5);
           translate([64,20.7,BACK_THICKNESS-0.1])
           cylinder(r=2, h=DEPTH-5);
            }
                
           translate([35,3,BACK_THICKNESS-0.1])
           rtc_mounts(9);

           // corner hook thing
            translate([0.3,-0.3,0])  // allow some clearance for touching side
            difference() {

           translate([HOOK_R+SIDE_THICKNESS+0.1, OUTER_HEIGHT-HOOK_R-SIDE_THICKNESS-0.1, BACK_THICKNESS-0.1])
           cylinder(r=HOOK_R, h=9);

        union() {
        hull() {
            translate([SIDE_THICKNESS-0.1, OUTER_HEIGHT-SIDE_THICKNESS+0.1, BACK_THICKNESS])
            cylinder(r=0.1, h=2);
            translate([SIDE_THICKNESS+1, OUTER_HEIGHT-SIDE_THICKNESS-1, BACK_THICKNESS+2])
            cylinder(r=2, h=1);
        }

        hull() {
            
            translate([SIDE_THICKNESS+1, OUTER_HEIGHT-SIDE_THICKNESS-1, BACK_THICKNESS+2])
            cylinder(r=2, h=1);
               
        translate([SIDE_THICKNESS-0.1, OUTER_HEIGHT-SIDE_THICKNESS+0.1, BACK_THICKNESS+3+1])
            cylinder(r=.1, h=2);
        
            }
        }
       }
   } // union
       
       translate([3,27,BACK_THICKNESS-0.1])
       pcb_holes();
       
       //pwr connector
       translate([60,12,-1])
       cylinder(r=4.5,h=10);
       
        // screw hole
       translate([0,0,1.3])
       mirror([0,0,1])
       translate([OUTER_WIDTH-HOLDER_OUTER_R, HOLDER_OUTER_R, -1])
       tapped_hole(r1=SCREW_R1, r2=SCREW_R2, h1=1.2, h2=1.3);

   }
}

//base();
back();
