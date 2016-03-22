/*
 Changes:
 
 - power connector has 11mm dia
 
 - tapered back
 
 - screwless mounts for pcbs (except for rotary encoder)

*/
use <logo.scad>;

$fn = 20;

SIDE_THICKNESS = 2;
FRONT_THICKNESS = 1;
BACK_THICKNESS = 2;

// 26 for cyclone pcb, 25 for eurocircuits pcb
DEPTH = 24.5 + FRONT_THICKNESS + BACK_THICKNESS;  

INNER_WIDTH = 66;
OUTER_WIDTH = INNER_WIDTH + 2*SIDE_THICKNESS;
INNER_HEIGHT = 66;
OUTER_HEIGHT = INNER_HEIGHT + 2*SIDE_THICKNESS;

ROUNDING_R = 3;

HOLDER_OUTER_R = 4;
HOLDER_INNER_R = 1.3;
HOLDER_H = 10;

// holder
HOOK_R = 2;
HOOK_SPHERE_R = 2;
SPHERE_SIDE_OFFSET = 1;
SPHERE_HEIGHT = 2.5;

echo("OUTER_WIDTH=");
echo(OUTER_WIDTH);
echo("OUTER_HEIGHT=");
echo(OUTER_HEIGHT);

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

// subtract this shape to have a nice fit for screwless pcb mounting.
module pcb_cutout(x, y, z=1.6) {
    translate([0,0,z-0.1])
    hull() {
        cube([x, y, 0.1]);
        
        translate([2, 2, 2])
        cube([x-4, y-4, 0.1]);
    }
    
    cube([x, y, z]);
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
    
    translate([OUTER_WIDTH - 0.7, OUTER_HEIGHT/2-7, 23])
    rotate([0,90,0])
    logo(5, 15);

    // holder
    translate([SPHERE_SIDE_OFFSET+SIDE_THICKNESS, SPHERE_SIDE_OFFSET+SIDE_THICKNESS, DEPTH-SPHERE_HEIGHT+0.2-BACK_THICKNESS])
    sphere(r=HOOK_SPHERE_R+0.15);

    translate([OUTER_WIDTH-(SPHERE_SIDE_OFFSET+SIDE_THICKNESS), SPHERE_SIDE_OFFSET+SIDE_THICKNESS, DEPTH-SPHERE_HEIGHT+0.2-BACK_THICKNESS])
    sphere(r=HOOK_SPHERE_R+0.15);

    translate([OUTER_WIDTH-(SPHERE_SIDE_OFFSET+SIDE_THICKNESS), OUTER_HEIGHT-(SPHERE_SIDE_OFFSET+SIDE_THICKNESS), DEPTH-SPHERE_HEIGHT+0.2-BACK_THICKNESS])
    sphere(r=HOOK_SPHERE_R+0.15);

    translate([SPHERE_SIDE_OFFSET+SIDE_THICKNESS, OUTER_HEIGHT-(SPHERE_SIDE_OFFSET+SIDE_THICKNESS), DEPTH-SPHERE_HEIGHT+0.2-BACK_THICKNESS])
    sphere(r=HOOK_SPHERE_R+0.15);

    // taper back side
    translate([2,2,DEPTH-BACK_THICKNESS])
    rounded_cube2(OUTER_WIDTH-4, OUTER_HEIGHT-4, BACK_THICKNESS+0.2, ROUNDING_R-2,
               OUTER_WIDTH, OUTER_HEIGHT, ROUNDING_R);

}

    translate([2-0.7, OUTER_HEIGHT/2-7, 4.5])
    rotate([0,-90,0])
    logo(2, 15);

    // grid in front, testing
    *for (i=[0:6]) {
        translate([-0.25+i*8.14+8.14+SIDE_THICKNESS+0.43, 1, 0.1])
        cube([0.5, OUTER_HEIGHT-2, FRONT_THICKNESS+0.5]);
    }
    *for (i=[0:6]) {
        if (i == 6) {
            translate([1, -0.25+i*8.31+8.31+SIDE_THICKNESS-0.25, 0.1])
            cube([OUTER_WIDTH-2-9.0, 0.5, FRONT_THICKNESS+0.5]);
        } else {
            translate([1, -0.25+i*8.31+8.31+SIDE_THICKNESS-0.25, 0.1])
            cube([OUTER_WIDTH-2, 0.5, FRONT_THICKNESS+0.5]);
        }
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
}

module pcb_enc_mount() {
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

MOUNTING_R = 1.5;
HOOK_CLEARANCE = 0.7;  // was 0.2

module back() {
   difference() {
       union() {
           //rounded_cube(OUTER_WIDTH, OUTER_HEIGHT, BACK_THICKNESS, ROUNDING_R);
           rounded_cube2(OUTER_WIDTH, OUTER_HEIGHT, BACK_THICKNESS, ROUNDING_R,
                OUTER_WIDTH-4, OUTER_HEIGHT-4, ROUNDING_R-2);
           
           translate([3,27,BACK_THICKNESS-0.1])
           pcb_enc_mount();

            // support for display
            hull() {
           translate([64,24.7,BACK_THICKNESS-0.1])
           cylinder(r=2, h=DEPTH-7);
           translate([64,20.7,BACK_THICKNESS-0.1])
           cylinder(r=2, h=DEPTH-7);
            }           

           hull() {
            translate([10,67.3-MOUNTING_R,FRONT_THICKNESS-0.1])
            cylinder(r=MOUNTING_R, h=13);

            translate([25,67.3-MOUNTING_R,FRONT_THICKNESS-0.1])
            cylinder(r=MOUNTING_R, h=13);
           }

           translate([64,27,FRONT_THICKNESS-0.1])
           cylinder(r=MOUNTING_R, h=13);

            hull() {
               translate([10,27,FRONT_THICKNESS-0.1])
               cylinder(r=MOUNTING_R, h=13);

               translate([32,27,FRONT_THICKNESS-0.1])
               cylinder(r=MOUNTING_R, h=13);
            }

            hull() {
               translate([10,SIDE_THICKNESS+0.7+MOUNTING_R,FRONT_THICKNESS-0.1])
               cylinder(r=MOUNTING_R, h=10);

               translate([32,SIDE_THICKNESS+0.7+MOUNTING_R,FRONT_THICKNESS-0.1])
               cylinder(r=MOUNTING_R, h=10);
            }
                
           // corner hook thing; clearance of HOOK_CLEARANCE
           translate([HOOK_R+SIDE_THICKNESS+0.1, OUTER_HEIGHT-HOOK_R-SIDE_THICKNESS-0.1, BACK_THICKNESS-0.1])
           cylinder(r=HOOK_R, h=SPHERE_HEIGHT);
            
           translate([SPHERE_SIDE_OFFSET+SIDE_THICKNESS+HOOK_CLEARANCE, OUTER_HEIGHT-SPHERE_SIDE_OFFSET-SIDE_THICKNESS-HOOK_CLEARANCE, BACK_THICKNESS+SPHERE_HEIGHT])
           sphere(r=HOOK_SPHERE_R);


           translate([OUTER_WIDTH-(HOOK_R+SIDE_THICKNESS+0.1), OUTER_HEIGHT-HOOK_R-SIDE_THICKNESS-0.1, BACK_THICKNESS-0.1])
           cylinder(r=HOOK_R, h=SPHERE_HEIGHT);
            
           translate([OUTER_WIDTH-(SPHERE_SIDE_OFFSET+SIDE_THICKNESS)-HOOK_CLEARANCE, OUTER_HEIGHT-SPHERE_SIDE_OFFSET-SIDE_THICKNESS-HOOK_CLEARANCE, BACK_THICKNESS+SPHERE_HEIGHT])
           sphere(r=HOOK_SPHERE_R);


           translate([OUTER_WIDTH-(HOOK_R+SIDE_THICKNESS+0.1), HOOK_R+SIDE_THICKNESS+0.1, BACK_THICKNESS-0.1])
           cylinder(r=HOOK_R, h=SPHERE_HEIGHT);
            
           translate([OUTER_WIDTH-(SPHERE_SIDE_OFFSET+SIDE_THICKNESS)-HOOK_CLEARANCE, SPHERE_SIDE_OFFSET+SIDE_THICKNESS+HOOK_CLEARANCE, BACK_THICKNESS+SPHERE_HEIGHT])
           sphere(r=HOOK_SPHERE_R);


           translate([HOOK_R+SIDE_THICKNESS+0.1, HOOK_R+SIDE_THICKNESS+0.1, BACK_THICKNESS-0.1])
           cylinder(r=HOOK_R, h=SPHERE_HEIGHT);
            
           translate([SPHERE_SIDE_OFFSET+SIDE_THICKNESS+HOOK_CLEARANCE, SPHERE_SIDE_OFFSET+SIDE_THICKNESS+HOOK_CLEARANCE, BACK_THICKNESS+SPHERE_HEIGHT])
           sphere(r=HOOK_SPHERE_R);
                        
                        
   } // union
       
       translate([3,27,BACK_THICKNESS-0.1])
       pcb_holes();
       
       //pwr connector
       translate([59,11.5,-1])
       cylinder(r=5.7,h=10, $fn=30);

       // pcb cutout
       //translate([SIDE_THICKNESS+0.5, OUTER_HEIGHT - SIDE_THICKNESS - 2 - 38, FRONT_THICKNESS + 9])
       translate([3, 27+0.2, FRONT_THICKNESS + 9])
       #pcb_cutout(64, 38.2);

       translate([SIDE_THICKNESS+0.5, 5-0.2, FRONT_THICKNESS + 6])
       #pcb_cutout(38, 22.2);
   
       translate([SIDE_THICKNESS+0.5+16.5, 5+11, FRONT_THICKNESS + 3])
       cylinder(r=12, h=10, $fn=30);
   }
}

base();
//back();

//pcb_cutout(100,100);