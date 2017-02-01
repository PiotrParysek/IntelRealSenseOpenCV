#include "Variables.h"

#include "IntelRealSense.h"

int main()
{
	bool view = false, hand = false, game = false, depthgestures = false;
	std::cout << "Choose mode" << std::endl;
	std::cout << "1 - camera view mode\n" <<
		"2 - hand data mode\n" <<
		"3 - depth and gestures\n" <<
		"4 - simple game" << std::endl;
	int X;
	std::cin >> X;
	switch (X)
	{
	case 1:
		view = true;
		break;
	case 2:
		hand = true;
		break;
	case 3:
		depthgestures = true;
		break;
	case 4:
		game = true;
		break;
	default:
		std::cout << "Wrong mode selected - Exit program!" << std::endl;
		return EXIT_SUCCESS;
		break;
	}
	if (view)
	{
		std::cout << "Choose what stream you want to see:\n"
			<< "1 - Color only\n"
			<< "2 - Depth only\n"
			<< "3 - Color and Depth\n"
			<< "4 - IR only\n"
			<< "5 - Color and IR\n"
			<< "6 - Depth and IR\n"
			<< "7 - Color, Depth and IR" << std::endl;
		std::cin >> X;
		if (X < 1 || X > 7) 
		{
			std::cout << "Wrong stream selected - Exit program!" << std::endl;
			return EXIT_FAILURE;
		}
		
		IntelRealSense *irs = new IntelRealSense();
		irs->setStreamType(X);

		cv::Mat color, depth, ir;
		std::cout << "Start Streaming to exit press [Esc] or [Space]: " << irs->start() << std::endl;
		while (true)
		{
			if (CHECK_BIT(X, 0) != 0)
			{
				irs->retrive(color, Variables::STREAM_TYPE_COLOR);
				cv::imshow("Color", color);
			} 
			if (CHECK_BIT(X, 1) != 0)
			{
				irs->retrive(depth, Variables::STREAM_TYPE_DEPTH);
				cv::imshow("depth", depth);
			}
			if (CHECK_BIT(X, 2) != 0)
			{
				irs->retrive(ir, Variables::STREAM_TYPE_IR);
				cv::imshow("ir", ir);
			}

			int key = cv::waitKey(1);
			if (key == 27 || key == 32) {
				std::cout << "END: " << irs->stop() << std::endl;
				break;
			}
		}

		irs->stop();

		while (!irs->isStopped());

		std::cout << "END!" << std::endl;
		std::cin.get();

		return EXIT_SUCCESS;
	}
	if (hand)
	{
		std::cout << "Choose what stream you want to use:"
			<< "1 - Hands only\n"
			<< "2 - Joints only\n"
			<< "3 - Hands and Joints\n"
			<< "4 - Full joints stream" << std::endl;
		std::cin >> X;
		if (X < 1 || X > 4)
		{
			std::cout << "Wrong stream selection - Exit program!" << std::endl;
			return EXIT_FAILURE;
		}
		
		IntelRealSense *irs = new IntelRealSense();
		bool gesture = false, joint = false;
		switch (X)
		{
		case 1:
			irs->setStreamType(Variables::STREAM_TYPE_HAND);
			gesture = true;
			break;
		case 2:
			irs->setStreamType(Variables::STREAM_TYPE_JOINTS);
			joint = true;
			break;
		case 3:
			irs->setStreamType(Variables::STREAM_TYPE_HAND | Variables::STREAM_TYPE_JOINTS);
			gesture = joint = true;
			break;
		case 4:
			irs->setStreamType(Variables::STREAM_TYPE_JOINTS);
			irs->setfullHandData(true);
			joint = true;
			break;
		}

		std::string gest = "", oldgest = "";
		std::string joints = "", oldjoints = "";
		std::cout << "Start Streaming to exit close console: " << irs->start() << std::endl;
		while (true)
		{
			if (gesture)
			{
				irs->retrive(gest, Variables::STREAM_TYPE_HAND);
				if (gest == "")
					continue;
				if (gest != oldgest)
				{
					std::cout << "GEST: " << gest << std::endl;
					oldgest = gest;
				}
			}
			if (joint)
			{
				irs->retrive(joints, Variables::STREAM_TYPE_JOINTS);
				if (joints == "")
					continue;
				if (joints != oldjoints)
				{
					std::cout << "JOINT: " << joints << std::endl;
					oldjoints = joints;
				}
			}
		}

		irs->stop();

		while (!irs->isStopped());

		std::cout << "END!" << std::endl;
		std::cin.get();

		return EXIT_SUCCESS;
	}
	if (game)
	{
		std::cout << "How many players play the game?:";
		std::cin >> X;
		if (X < 1)
		{
			std::cout << "Sb has to play the game..." << std::endl;
			return EXIT_FAILURE;
		}

		std::cout << "How much time does each player has? [seconds]" << std::endl;
		int time;
		std::cin >> time;
		if (time < 10)
		{
			std::cout << "To sort amount of time at least 10 seconds!" << std::endl;
			return EXIT_FAILURE;
		}
		int *players = new int[X];
		for (size_t i = 0; i < X; i++)
			players[i] = 0;
		
		std::string gest = "", oldgest = "";
		std::string GESTURES[] = {
			"fist",
			"full_pinch",
			"spreadfingers",
			"thumb_down",
			"thumb_up",
			"two_fingers_pinch_open",
			"v_sign"
		};
		std::srand((unsigned int) std::time(NULL));
		std::string fin_gest;
		IntelRealSense *irs = new IntelRealSense();
		irs->setStreamType(Variables::STREAM_TYPE_HAND);
		std::cout << "Start THE GAME! " << irs->start() << std::endl;
		for (int i = 0; i < X; i++)
		{
			std::cout << "Player " << i << " starts in 3 seconds!" << std::endl;
			std::chrono::seconds dura(3);
			std::this_thread::sleep_for(dura);

			std::cout << "GO!" << std::endl;
			std::clock_t begin = std::clock();
			bool nextGEST = true;
			while (((std::clock() - begin) / CLOCKS_PER_SEC) < time)
			{
				fin_gest = GESTURES[rand() % 7];
				std::cout << "MAKE: " << fin_gest << std::endl;
				nextGEST = true;
				while (nextGEST)
				{
					irs->retrive(gest, Variables::STREAM_TYPE_HAND);
					if (gest == "")
						continue;
					if (gest != oldgest)
					{
						oldgest = gest;
						if (gest == fin_gest)
						{
							players[i]++;
							nextGEST = false;
						}
					}
				}
			}
			std::cout << "Resoult: " << players[i] << " gestures" << std::endl;
		}
		irs->stop();

		while (!irs->isStopped());

		std::cout << "RESOULTS: " << std::endl;
		for (size_t i = 0; i < X; i++)
			std::cout << "PLAYER: " << i << " SCORE: " << players[i] << std::endl;
		std::cout << "CONGRATULATIONS" << std::endl;

		delete players;

		std::cout << "END!" << std::endl;
		std::cin.get();
	}
	
	if (depthgestures) {
		IntelRealSense *irs = new IntelRealSense();
		irs->setStreamType(Variables::STREAM_TYPE_HAND | Variables::STREAM_TYPE_COLOR);
		std::cout << "Start capture " << irs->start() << std::endl;
		cv::Mat color;
		std::string gest = "", oldgest = "";
		while (true)
		{
			irs->retrive(color, Variables::STREAM_TYPE_COLOR);
			cv::imshow("color", color);

			irs->retrive(gest, Variables::STREAM_TYPE_HAND);
			if (gest == "")
				continue;
			if (gest != oldgest)
			{
				std::cout << "GEST: " << gest << std::endl;
				oldgest = gest;
			}

			int key = cv::waitKey(1);
			if (key == 27 || key == 32) {
				std::cout << "END: " << irs->stop() << std::endl;
				break;
			}
		}
	}
	std::cout << "END!" << std::endl;
	std::cin.get();

	return EXIT_SUCCESS;
}
