// STAHL CASE
// Copyright (c) 2021 Lone Dynamics Corporation

$fn = 30;

box_width = 18;
box_length = 26;
box_height = 8.5;
box_thickness = 2.0;

lid_thickness = 1.5;

cutout_width = 16.10;
cutout_length = 24.10;
cutout_height = 7;

cutout_usb_width = 12.5;

board_height = 1.4;

ldp_case();
ldp_board(-1);

//color([0.8,0.8,0.8]) ldp_lid(6);
color([0.8,0.8,0.8]) ldp_lid(2);

module ldp_board(o)
{
	translate([0,0,o])
		color([0.0,0.8,0.0]) cube([16, 24, 1.6], center=true);
	translate([0,19.5-4,o+1.45])
		color([0.9,0.9,0.9]) cube([12, 15, 4.5], center=true);
	
}

module ldp_case()
{

	difference() {
		
		cube([box_width,box_length,box_height], center=true);
		
		translate([0,0,0.5])
			cube([cutout_width,cutout_length,cutout_height+1], center=true);

		translate([0,15,3])
			cube([cutout_usb_width,10,10], center=true);

		translate([box_width/2,box_length/2-5,box_height/2-1.125])
			cube([2,2,1], center=true);
		translate([box_width/2,-box_length/2+5,box_height/2-1.125])
			cube([2,2,1], center=true);
		translate([-box_width/2,box_length/2-5,box_height/2-1.125])
			cube([2,2,1], center=true);
		translate([-box_width/2,-box_length/2+5,box_height/2-1.125])
			cube([2,2,1], center=true);

	}

	// board support
	translate([0,-3,-2.75])
		cube([8,8,1.5], center=true);


}

module ldp_lid(o)
{
	
	difference() {
		
		union() {
			
			translate([0,0,o+1.5])
				cube([box_width-2,box_length-2,lid_thickness], center=true);

			translate([0,box_length/2-1,o+1.5])
				cube([12,2,lid_thickness], center=true);

			translate([box_width/2-0.5,box_length/2-5,o+1.125])
				cube([1.25,1.75,0.75], center=true);
			translate([box_width/2-0.5,-box_length/2+5,o+1.125])
				cube([1.25,1.75,0.75], center=true);
			translate([-box_width/2+0.5,box_length/2-5,o+1.125])
				cube([1.25,1.75,0.75], center=true);
			translate([-box_width/2+0.5,-box_length/2+5,o+1.125])
				cube([1.25,1.75,0.75], center=true);
		}
	
		rotate(270)
			translate([0.5,-1.5,o+1.5])
				linear_extrude(1)
					text("STAHL", size=3, spacing=1.75, halign="center");
	
	}
	
}