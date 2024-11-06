#include "main.h" // IWYU pragma: keep
#include <iterator>
	#include "lemlib/api.hpp" // IWYU pragma: keep
	#include "lemlib/chassis/chassis.hpp"
	#include "lemlib/chassis/trackingWheel.hpp"
	#include "lemlib/pid.hpp" // IWYU pragma: keep
	#include "lemlib/pose.hpp"
	#include "liblvgl/core/lv_disp.h" // IWYU pragma: keep
	#include "liblvgl/core/lv_obj.h" // IWYU pragma: keep
	#include "liblvgl/core/lv_obj_pos.h" // IWYU pragma: keep
	#include "liblvgl/core/lv_obj_style.h" // IWYU pragma: keep
	#include "liblvgl/core/lv_obj_tree.h" // IWYU pragma: keep
	#include "liblvgl/misc/lv_color.h" // IWYU pragma: keep
	#include "liblvgl/misc/lv_style.h" // IWYU pragma: keep
	#include "liblvgl/widgets/lv_img.h" // IWYU pragma: keep
	#include "pros/abstract_motor.hpp" // IWYU pragma: keep
	#include "pros/adi.hpp" // IWYU pragma: keep
	#include "pros/colors.hpp" // IWYU pragma: keep
	#include "pros/distance.hpp" // IWYU pragma: keep
	#include "pros/llemu.hpp" // IWYU pragma: keep
	#include "pros/misc.h" // IWYU pragma: keep
	#include "pros/misc.hpp"
	#include "pros/motor_group.hpp" // IWYU pragma: keep
	#include "pros/motors.h"
	#include "pros/motors.hpp" // IWYU pragma: keep
	#include <cstdio> // IWYU pragma: keep
	#include <cwchar> // IWYU pragma: keep
	#include <string> // IWYU pragma: keep
	#include "as.h" // IWYU pragma: keep
	#include "pros/optical.hpp" // IWYU pragma: keep
	#include "pros/rotation.hpp" // IWYU pragma: keep
	#include "pros/rtos.hpp"
	#include "pros/vision.hpp" // IWYU pragma: keep


// LemLib Setup
pros::MotorGroup right_motors({20,19,17}, pros::MotorGearset::blue);
pros::MotorGroup left_motors({-18,-16,-15}, pros::MotorGearset::blue);
pros::Imu imu(12);
pros::Motor LadyBrown(14);
pros::Motor intake(13);
pros::Rotation rotation_sensor(1);
pros::adi::Pneumatics clamp ('E', false);
pros::adi::Pneumatics corner_clear ('B', false);
pros::adi::Pneumatics intake_lift ('A', false);


/*
	pros::Rotation vert(4);
	lemlib::TrackingWheel vert_tracking(&vert, 2.125, 1.875, 1.0);
	pros::Rotation horiz(5);
	lemlib::TrackingWheel horiz_tracking(&horiz, 2.125, 0.375, 1.0);
*/

float track_width = 13;								//track width is 12.75 inches
float wheel_diameter = lemlib::Omniwheel::NEW_325;		//bot uses the new 3.25 inch wheels
float rpm = 450.0;										//RPM of robot is 450
float horizontal_drift = 2.0;							//Not sure yet.

// drivetrain controller
lemlib::Drivetrain drivetrain(
	&left_motors,
	&right_motors,
	track_width,
	wheel_diameter,
	rpm,
	horizontal_drift
);

// sensors controller
lemlib::OdomSensors sensors(
	nullptr, // vertical tracking wheel 1, set to null
	nullptr, // vertical tracking wheel 2, set to nullptr as we are using IMEs
	nullptr, // horizontal tracking wheel 1
	nullptr, // horizontal tracking wheel 2, set to nullptr as we don't have a second one
	&imu // inertial sensor
);

// lateral PID controller
lemlib::ControllerSettings lateral_controller(
	10, // proportional gain (kP)15
	0, // integral gain (kI)
	3, // derivative gain (kD)20
	3, // anti windup0
	1, // small error range, in inches0.1
	100, // small error range timeout, in milliseconds
	3, // large error range, in inches1
	500, // large error range timeout, in milliseconds
	20 // maximum acceleration (slew)
);

// angular PID controllerk
lemlib::ControllerSettings angular_controller(
	2, // proportional gain (kP)15
	0, // integral gain (kI)
	10, // derivative gain (kD)30
	3, // anti windup0
	1, // small error range, in degrees0.5
	100, // small error range timeout, in milliseconds
	3, // large error range, in degrees1
	500, // large error range timeout, in milliseconds
	0 // maximum acceleration (slew)try 8 maybe later
);


// input curve for throttle input during driver control
lemlib::ExpoDriveCurve 
throttleCurve(3,    // joystick deadband out of 127
             10,   // minimum output where drivetrain will move out of 127
                 1.019 // expo curve gain
);

// input curve for steer input during driver control
lemlib::ExpoDriveCurve 
steerCurve(3,    // joystick deadband out of 127
          10,   // minimum output where drivetrain will move out of 127
              1.019 // expo curve gain
);



// chassis controller
lemlib::Chassis chassis(
	drivetrain, // drivetrain settings
	lateral_controller, // lateral PID settings
	angular_controller, // angular PID settings
	sensors, // odometry sensors
	&throttleCurve,
	&steerCurve

);

// Devices
pros::Controller controller(pros::E_CONTROLLER_MASTER);
pros::adi::DigitalIn auton_btn('f');

void auton_init();

void initialize() {
	lcd::initialize();
    chassis.calibrate(); // calibrate sensors
	auton_init();

	pros::c::motor_set_brake_mode(14, pros::E_MOTOR_BRAKE_BRAKE);


	pros::Task as_button([&]() {
		bool new_press = true;
		
		while (true) {
			if (pros::competition::is_disabled()) {
				if (auton_btn.get_value() == 1) {
					if (new_press) {
						as::increment();
						new_press = false;
					}
				} else {
					new_press = true;
				}
			} else {
				break;
			}
			pros::delay(20);
		}
	});



	// REMOVE THIS WHEN AUTONS ARE DONE
	//LITENNNNNNNNNNNNNNNNNNN
	
	pros::Task screen_task([&]() {
	  	while (true) {
            // print robot location to the brain screen
            lcd::print(0, "X: " + std::to_string(chassis.getPose().x)); // x
            lcd::print(1, "Y: " + std::to_string(chassis.getPose().y)); // y
            lcd::print(2, "Theta: " + std::to_string(chassis.getPose().theta)); // heading
			//lcd::print(3, "Angle: " + std::to_string(rotation_sensor.get_angle())); // heading
			//lcd::print(4, "Position: " + std::to_string(rotation_sensor.get_position())); // heading
			//lcd::print(5, std::to_string(rotation_sensor.get_angle())); // heading

			// delay to save resources
            pros::delay(20);
        }
    });


}

void disabled() {
}

void competition_initialize() {
}

void autonomous() {
	lcd::clear();
	
	pros::Task screen_task([&]() {
	  	while (true) {
            // print robot location to the brain screen
            lcd::print(0, "X: " + std::to_string(chassis.getPose().x)); // x
            lcd::print(1, "Y: " + std::to_string(chassis.getPose().y)); // y
            lcd::print(2, "Theta: " + std::to_string(chassis.getPose().theta)); // heading

            lemlib::telemetrySink()->info("Chassis pose: {}", chassis.getPose());

			// delay to save resources
            pros::delay(20);
        }
    });

	chassis.setBrakeMode(pros::E_MOTOR_BRAKE_HOLD);
	chassis.setPose(lemlib::Pose(0, 0, 0), false);
	as::call_selected_auton();
}

void opcontrol() {
	//lcd::clear();

	chassis.setBrakeMode(pros::E_MOTOR_BRAKE_COAST);

	pros::c::motor_set_brake_mode(14, pros::E_MOTOR_BRAKE_BRAKE);

	rotation_sensor.reset_position();
	rotation_sensor.set_position(0);

	while (true) {

		// get left y and right x positions
		int Y = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
		int X = -controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);


		// move the robot
		chassis.arcade(Y, -X);


		// activate the clamp
		if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_A)) {
			clamp.toggle();}


		//Activates corner clear
		if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_B)) {
			corner_clear.toggle();}
		

		//Activate intake life
		if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_Y)) {
			intake_lift.toggle();}
		

		// activate intake
		if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_L1)) {
			intake.move(-127);}
		else if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_L2)) {
			intake.move(127);}
		else {
			intake.move(0);}


		// activate lady brown
		if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) {
			LadyBrown.move(50);}
		else if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {
			LadyBrown.move(-50);}
		else {
			LadyBrown.move(0);}


		// Save from horizontal limit
		if ((rotation_sensor.get_position()) >= 56000){
			LadyBrown.move(80);}


		// Press to line up
		if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_X)) {
			if (rotation_sensor.get_position() > 2000){
				LadyBrown.move(80); //goes down
			}
			else if ((rotation_sensor.get_position() < -2000)) {
				LadyBrown.move(-80); //goes up
			}
			else if ((rotation_sensor.get_position() > -2000 && (rotation_sensor.get_position() < 2000))) {
				LadyBrown.move(0);}}


		// delay to keep tasks running
		pros::delay(10);
	}
}

/*

	AUTON FUNCTIONS

*/
void auton_1() {
	// auton code here

	// BLUE NEGATIVE AUTON

	chassis.setBrakeMode(pros::E_MOTOR_BRAKE_HOLD);


		chassis.moveToPoint(0, -25, 3000, {.forwards = false}, false);

		chassis.turnToHeading(45, 1000);

		chassis.moveToPoint(-24, -46, 3000, {.forwards = false}, false);

		chassis.turnToHeading(0, 1000);

		chassis.moveToPose(-12, -79, -39, 3000,{.forwards = false}, false);



	
/*
	// Push the lady brown up max speed for half a sec
		LadyBrown.move(-127);
		pros::delay(200);
		LadyBrown.move(0);
		pros::delay(200);





		//turn to first ring
		intake_lift.toggle();
		chassis.turnToHeading(-58.88, 1000);
	    chassis.waitUntilDone();

		// move to first ring and pick it up
		chassis.moveToPoint(-8.789, 3.369, 1000, {.forwards = true});
		intake.move(-127);
		chassis.waitUntilDone();


		//stop intake after pickup also drive more forward for lineup
		pros::delay(300); // this delays intake ending
		intake.move(0);
		intake_lift.toggle();
		pros::delay(100);
		chassis.turnToHeading(25, 2000, {.maxSpeed = 10},false);

		chassis.moveToPose(-20, -6,-0,  3000,{.forwards= false}, false);
		//arm go down
		LadyBrown.move(127);
		pros::delay(500);
		LadyBrown.move(0);
		chassis.moveToPose(6, 23,-146,  4000,{.forwards= false});
		clamp.toggle();
*/
/* //-13, 0, 7

		//goes back
		chassis.moveToPoint(0, 0, 2000, {.forwards = false});
		chassis.waitUntilDone();

		//turns to red 
		chassis.turnToHeading(-98, 1000);
		chassis.waitUntilDone();

		chassis.moveToPose(-13, -8.6, 0, 3000, {.forwards = false});
		chassis.waitUntilDone();

		

		//gets out of stake area
		chassis.moveToPoint(-11, 3, 2000, {.forwards = true});
		chassis.waitUntilDone();



		//drive to mogo
		chassis.moveToPose(6, 16, -501, 4000, {.forwards = false});
		chassis.waitUntilDone();

		//clamp
		clamp.toggle();
		//get clamp
		*/
		/*
		chassis.moveToPose(12, 27.7, -492, 4000, {.forwards = false});
		chassis.waitUntilDone();

		
*/

/* 12, 27.7, -492
		//pushes red out of way
		chassis.moveToPoint(-15.18, -2.9, 2000, {.forwards = true});
		chassis.waitUntilDone();
*/


		//line up with wall stake
		//chassis.moveToPose(float x, float y, float theta, int timeout) //find out where it needs to move to

	/*
		chassis.moveToPoint(-15.765, 8.77, 1000, {.forwards = true});	
		chassis.waitUntilDone();

		// Turns to alliance stake
		chassis.turnToHeading(0, 1000);
		chassis.waitUntilDone();*/
/*
		// Drives to the stake and scores
		chassis.moveToPoint(-16.679, -4.566, 1000, {.forwards = false});	
		chassis.waitUntilDone();
		LadyBrown.move(127);
		pros::delay(500);
		LadyBrown.move(0);

		//Drives out of the area
		chassis.moveToPoint(-16.36, 7.323, 1000, {.forwards = true});
		chassis.waitUntilDone();

		//move to pose of mogo
		chassis.moveToPose(5.671, 22.88, -140.44, 4000, {.forwards = false});
		clamp.toggle();
		//chassis.turnToHeading(-12, 1000);
*/






	// Drives to set up position for stake from start point

	/*
	chassis.moveToPoint(0, -14.544, 4000, {.forwards = false});
    chassis.waitUntilDone();



	// Turns to face stake
	chassis.turnToHeading(-87.82, 400);
    chassis.waitUntilDone();


	// Drives to stake, scores
	chassis.moveToPoint(5.033, -15.96, 4000, {.forwards = false});
	chassis.waitUntilDone();
	// run intake to score on stake
*/

}

void auton_init() {
	as::add_auton(as::Auton("Auton 1", "Blue Negative Side Auton", auton_1));



	as::init();
}
