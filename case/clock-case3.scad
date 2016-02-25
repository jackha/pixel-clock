use <logo.scad>;

$fn = 20;

SIDE_THICKNESS = 2;
FRONT_THICKNESS = 1;
BACK_THICKNESS = 2;

DEPTH = 26 + FRONT_THICKNESS;

INNER_WIDTH = 66;
OUTER_WIDTH = INNER_WIDTH + 2*SIDE_THICKNESS;
INNER_HEIGHT = 66;
OUTER_HEIGHT = INNER_HEIGHT + 2*SIDE_THICKNESS;

ROUNDING_R = 2;

HOLDER_OUTER_R = 4;
HOLDER_INNER_R = 1.3;
HOLDER_H = 10;

// holder
HOOK_R = 2;
HOOK_SPHERE_R = 2;
SPHERE_SIDE_OFFSET = 1;
SPHERE_HEIGHT = 2.5;


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

    // holder
           translate([SPHERE_SIDE_OFFSET+SIDE_THICKNESS, SPHERE_SIDE_OFFSET+SIDE_THICKNESS, DEPTH-SPHERE_HEIGHT+0.2])
           sphere(r=HOOK_SPHERE_R+0.15);

           translate([OUTER_WIDTH-(SPHERE_SIDE_OFFSET+SIDE_THICKNESS), OUTER_HEIGHT-(SPHERE_SIDE_OFFSET+SIDE_THICKNESS), DEPTH-SPHERE_HEIGHT+0.2])
           sphere(r=HOOK_SPHERE_R+0.15);

}

    translate([2-0.7, OUTER_HEIGHT/2-7, 8])
    rotate([0,-90,0])
    logo(2, 12);

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
           translate([HOOK_R+SIDE_THICKNESS+0.1, OUTER_HEIGHT-HOOK_R-SIDE_THICKNESS-0.1, BACK_THICKNESS-0.1])
           cylinder(r=HOOK_R, h=SPHERE_HEIGHT);
            
           translate([SPHERE_SIDE_OFFSET+SIDE_THICKNESS, OUTER_HEIGHT-SPHERE_SIDE_OFFSET-SIDE_THICKNESS, BACK_THICKNESS+SPHERE_HEIGHT])
           sphere(r=HOOK_SPHERE_R);

           translate([OUTER_WIDTH-(HOOK_R+SIDE_THICKNESS+0.1), HOOK_R+SIDE_THICKNESS+0.1, BACK_THICKNESS-0.1])
           cylinder(r=HOOK_R, h=SPHERE_HEIGHT);
            
           translate([OUTER_WIDTH-(SPHERE_SIDE_OFFSET+SIDE_THICKNESS), SPHERE_SIDE_OFFSET+SIDE_THICKNESS, BACK_THICKNESS+SPHERE_HEIGHT])
           sphere(r=HOOK_SPHERE_R);
                        
                        
   } // union
       
       translate([3,27,BACK_THICKNESS-0.1])
       pcb_holes();
       
       //pwr connector
       translate([60,12,-1])
       cylinder(r=4.5,h=10);
       
   }
}

//base();
back();
