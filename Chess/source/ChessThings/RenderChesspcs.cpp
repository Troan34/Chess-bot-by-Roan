#include "RenderChesspcs.h"
static MouseInput g_mouseInput;

static float RememberTexID;
static std::array<unsigned int, 64Ui64> static_BoardSquare;
static bool wasStatic_BoardSquareCreated = false;
static std::array<unsigned int, 64Ui64> previousBoardsquare;
static bool wasStatic_previousBoardsquareCreated = false;

//(mostly) Drop fun vars
static unsigned int MoveNum = 0;
static canCastle CanCastle;
static int BoardSquareBeingSelected = -1;
int AttackedSquare = -1;
std::unordered_set<unsigned int> LegalMovesForSelectedSquare;
static bool WaitingForUserPromotion = false;
static uint8_t rememberAttackedSquareForUserPromotion;

//multithreading vars
static std::atomic<bool> IsGetCommandRunning = true;
static std::string Command;
static std::atomic<bool> ReceivedACommand(false);


RenderChessPieces::RenderChessPieces()
{

}
RenderChessPieces::~RenderChessPieces()
{

}

static void cursorPositionCallBack(GLFWwindow* window, double xPosition, double yPosition)
{
	g_mouseInput.xPos = (float)xPosition;
	g_mouseInput.yPos = (float)yPosition;
}

static void mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods)
{
	

	if (button == GLFW_MOUSE_BUTTON_LEFT and action == GLFW_PRESS)
	{
		g_mouseInput.LeftButtonPressed = true;
	}
	else if (action != GLFW_PRESS)
	{
		g_mouseInput.LeftButtonPressed = false;
	}
}

//waits for a command to be written
static void GetCommand()
{
	std::getline(std::cin, Command);
	ReceivedACommand = true;
	IsGetCommandRunning = false;
}

static std::thread CommandThread(GetCommand);

/*********MAIN FUNCTION*********does quad calculations, calls command thread, drag and drop...**************/
std::array<std::array<VertexStructure, 4Ui64>, 135> RenderChessPieces::CreateObjects()
{
	quads[0] = CreateQuad(-350.0f, -350.0f, 700.0f, 0.0f);
	float xDifference = 0.0f;
	float yDifference = 0.0f;


	//Console Commands and threads
	if (!IsGetCommandRunning)
	{
		CommandThread.join();
		CommandThread = std::thread(GetCommand);
		IsGetCommandRunning = true;
	}

	if (ReceivedACommand == true)
	{
		if (Command == "help")
		{
			std::cout << "perft {depth}" << '\n' <<
				"other things\n";
		}
		else if (Command.find("perft") != std::string::npos)
		{
			uint16_t strIndex = Command.find("perft");

			char depth = Command.at(strIndex + 6);
			if (!isdigit(depth))
			{
				std::cout << "Wrong syntax" << std::endl;
			}
			else
			{
				depth = (uint8_t)depth - '0';
				void_Futures.push_back(std::async(std::launch::async, &RenderChessPieces::CreatePerft, this, depth));
				//std::cout << "Main CPU: " << GetCurrentProcessorNumber() << std::endl;
			}
		}
		else
		{
			std::cout << "Wrong syntax" << std::endl;
		}

		if (!IsGetCommandRunning)
		{
			CommandThread.join();
			CommandThread = std::thread(GetCommand);
			IsGetCommandRunning = true;
		}
		ReceivedACommand = false;
	}

	bool isNextMoveForWhite = true;
	if (MoveNum % 2 != 0)
		isNextMoveForWhite = false;


	//Legal Moves
	if (BoardSquareBeingSelected != -1)
	{
		float xxDifference = 0.0f;
		float yyDifference = 0.0f;
		static_BoardSquare[BoardSquareBeingSelected] = GetPieceTypefromTexID(RememberTexID);

		if (wasStatic_previousBoardsquareCreated)
		{
			GenerateLegalMoves LegalMoves(static_BoardSquare, &previousBoardsquare, CanCastle, isNextMoveForWhite, MoveNum, false);
			for (uint8_t j : LegalMoves.moves[BoardSquareBeingSelected].TargetSquares)
			{
				xxDifference = j * 87.5f;
				yyDifference = 0;
				while (xxDifference >= 650.0f)
				{
					xxDifference -= 700.0f;
					yyDifference += 87.5f;
				}
				quads[j + 66] = CreateQuad(-350.0f + xxDifference, -350.0f + yyDifference, 87.5f, 14);
				LegalMovesForSelectedSquare.insert(j);
			}
			
		}
		else
		{
			GenerateLegalMoves LegalMoves(static_BoardSquare,nullptr, CanCastle, isNextMoveForWhite, MoveNum, false);
			for (int j : LegalMoves.moves[BoardSquareBeingSelected].TargetSquares)
			{
				xxDifference = j * 87.5f;
				yyDifference = 0;
				while (xxDifference >= 650.0f)
				{
					xxDifference -= 700.0f;
					yyDifference += 87.5f;
				}
				quads[j + 66] = CreateQuad(-350.0f + xxDifference, -350.0f + yyDifference, 87.5f, 14);
				LegalMovesForSelectedSquare.insert(j);
			}

		}
		
		static_BoardSquare[BoardSquareBeingSelected] = 0;
	}


	//rendering part
 	for (int i = 1; i < 65; i++)
	{
		if (xDifference > 650.0f)
		{
			xDifference = 0.0f;
			yDifference += 87.5f;
		}

		float PieceTexID = GetPieceTextureID(static_BoardSquare, i - 1);
		bool HasDragAndDropFunHappened = false;

		//drag and drop and promotion
		if (g_mouseInput.LeftButtonPressed and g_mouseInput.WasLeftButtonPressed == false and PieceTexID != 0.0f)
		{
			if (!WaitingForUserPromotion)
			{
				//next two ifs are to check which objects have to be dragged
				if ((g_mouseInput.xPos - 350.0f) > (-350.0f + xDifference) and (g_mouseInput.xPos - 350.0f) < (-350.0f + xDifference + 87.5f))
				{
					if ((-g_mouseInput.yPos + 360.0f) > (-350.0f + yDifference) and (-g_mouseInput.yPos + 360.0f) < (-350.0f + yDifference + 87.5f))
					{

						quads[65] = CreateQuad(g_mouseInput.xPos - 393.5f, -g_mouseInput.yPos + 319.25f, 87.5f, PieceTexID);
						RememberTexID = PieceTexID;
						static_BoardSquare[i - 1] = (unsigned int)0;
						BoardSquareBeingSelected = i - 1;

					}
				}
			}
		}
		
		//this if is to "drop" the object ( and other things )
		if (g_mouseInput.LeftButtonPressed == false and g_mouseInput.WasLeftButtonPressed == true and !WaitingForUserPromotion and BoardSquareBeingSelected != -1)
		{
			if ((g_mouseInput.xPos - 350.0f) > (-350.0f + xDifference) and (g_mouseInput.xPos - 350.0f) < (-350.0f + xDifference + 87.5f))
			{
				if ((-g_mouseInput.yPos + 360.0f) > (-350.0f + yDifference) and (-g_mouseInput.yPos + 360.0f) < (-350.0f + yDifference + 87.5f))
				{
					if (RememberTexID != 13.0f and RememberTexID != 0.0f)
					{
						//the if under here is to check if the move is legal
						if (LegalMovesForSelectedSquare.find(i - 1) != LegalMovesForSelectedSquare.end())
						{
							if(isNextMoveForWhite == Board::IsPieceColorWhite(GetPieceTypefromTexID(RememberTexID)))
							{
								static_BoardSquare[BoardSquareBeingSelected] = GetPieceTypefromTexID(RememberTexID);
								previousBoardsquare = static_BoardSquare;
								static_BoardSquare[BoardSquareBeingSelected] = 0;
								static_BoardSquare[i - 1] = GetPieceTypefromTexID(RememberTexID);
								PieceTexID = RememberTexID;
								if (i != BoardSquareBeingSelected + 1)
								{
									wasStatic_previousBoardsquareCreated = true;
									MoveNum++;
								}
								AttackedSquare = i - 1;

								//castling and en passant
								if (AttackedSquare != -1)
								{
									if (RememberTexID == 12.0f or RememberTexID == 6.0f)
									{
										if (abs(BoardSquareBeingSelected - AttackedSquare) == 2)
										{
											if (BoardSquareBeingSelected - AttackedSquare == -2)
											{
												if (BoardSquareBeingSelected == 4)
												{
													static_BoardSquare[5] = 20;
													static_BoardSquare[7] = 0;
												}
												else
												{
													static_BoardSquare[61] = 12;
													static_BoardSquare[63] = 0;
												}
											}
											if (BoardSquareBeingSelected - AttackedSquare == 2)
											{
												if (BoardSquareBeingSelected == 4)
												{
													static_BoardSquare[3] = 20;
													static_BoardSquare[0] = 0;
												}
												else
												{
													static_BoardSquare[59] = 12;
													static_BoardSquare[56] = 0;
												}
											}
										}
									}

									//white en passant
									if (RememberTexID == 7)
									{
										if (previousBoardsquare[AttackedSquare] == 0)
										{
											if (BoardSquareBeingSelected - AttackedSquare == -7 or BoardSquareBeingSelected - AttackedSquare == -9)
											{
												static_BoardSquare[AttackedSquare - 8] = 0;
											}
										}
									}
									//black en passant
									if (RememberTexID == 1)
									{
										if (previousBoardsquare[AttackedSquare] == 0)
										{
											if (BoardSquareBeingSelected - AttackedSquare == 7 or BoardSquareBeingSelected - AttackedSquare == 9)
											{
												static_BoardSquare[AttackedSquare + 8] = 0;
											}
										}
									}
								}
								WillCanCastleChange(previousBoardsquare[BoardSquareBeingSelected], BoardSquareBeingSelected, AttackedSquare, CanCastle);

								//promoting
								if (AttackedSquare >= 56 and GetPieceTypefromTexID(RememberTexID) == 17 or AttackedSquare <= 7 and GetPieceTypefromTexID(RememberTexID) == 9 )
								{
									WaitingForUserPromotion = true;
									MoveNum--;
									rememberAttackedSquareForUserPromotion = AttackedSquare;
								}

								BoardSquareBeingSelected = -1;
								LegalMovesForSelectedSquare.clear();

							}
							else
							{
								static_BoardSquare[BoardSquareBeingSelected] = GetPieceTypefromTexID(RememberTexID);
								BoardSquareBeingSelected = -1;
							}
						}
						else
						{
							static_BoardSquare[BoardSquareBeingSelected] = GetPieceTypefromTexID(RememberTexID);
							BoardSquareBeingSelected = -1;
						}
						
					}

				}
			}
		}

		if (g_mouseInput.LeftButtonPressed and g_mouseInput.WasLeftButtonPressed and RememberTexID != 13.0f and !WaitingForUserPromotion)
		{
			quads[65] = CreateQuad(g_mouseInput.xPos - 393.5f, -g_mouseInput.yPos + 319.25f, 87.5f, RememberTexID); //took way too much time,
			//func to get the y from yPos is f(x) = -x + 360
		}

		quads[i] = CreateQuad(-350.0f + xDifference, -350.0f + yDifference, 87.5f, PieceTexID);
		
		xDifference += 87.5f;


	}

	//does promotion things
	if(WaitingForUserPromotion)
	{
		if (isNextMoveForWhite)//white promotion quads
		{
			quads[130] = CreateQuad(-87.5f, 0.0f, 175.0f, 15.0f);
			quads[131] = CreateQuad(-87.5f, 0.0f, 87.5f, 11.0f);
			quads[132] = CreateQuad(0.0f, 0.0f, 87.5f, 10.0f);
			quads[133] = CreateQuad(-87.5f, 87.5f, 87.5f, 9.0f);
			quads[134] = CreateQuad(0.0f, 87.5f, 87.5f, 8.0f);
		}
		else//black promotion quads
		{
			quads[130] = CreateQuad(-87.5f, -175.0f, 175.0f, 15.0f);
			quads[131] = CreateQuad(-87.5f, -175.0f, 87.5f, 6.0f);
			quads[132] = CreateQuad(0.0f, -175.0f, 87.5f, 4.0f);
			quads[133] = CreateQuad(-87.5f, -87.5f, 87.5f, 3.0f);
			quads[134] = CreateQuad(0.0f, -87.5f, 87.5f, 2.0f);
		}

		//check which promotion type was selected, the code is HIGHLY unreadable, comments may help you
		if (g_mouseInput.LeftButtonPressed and g_mouseInput.WasLeftButtonPressed == false)
		{
			if (isNextMoveForWhite)//when white promotes
			{
				if (-g_mouseInput.yPos + 360.0f < 87.5f and -g_mouseInput.yPos + 360.0f >= 0.0f)//the 'bottom' ones
				{
					if (g_mouseInput.xPos - 350.0f < 0.0f and g_mouseInput.xPos - 350.0f > -87.5f)//Q
					{
						static_BoardSquare[rememberAttackedSquareForUserPromotion] = 21;
						WaitingForUserPromotion = false;
						MoveNum++;
					}
					else if (g_mouseInput.xPos - 350.0f < 87.5f and g_mouseInput.xPos - 350.0f > 0.0f)//R
					{
						static_BoardSquare[rememberAttackedSquareForUserPromotion] = 20;
						WaitingForUserPromotion = false;
						MoveNum++;
					}
				}
				else if (-g_mouseInput.yPos + 360.0f <= 175.0f and -g_mouseInput.yPos + 360.0f >= 87.5f)//the 'top' ones
				{
					if (g_mouseInput.xPos - 350.0f < 0.0f and g_mouseInput.xPos - 350.0f > -87.5f)//N
					{
						static_BoardSquare[rememberAttackedSquareForUserPromotion] = 19;
						WaitingForUserPromotion = false;
						MoveNum++;
					}
					else if (g_mouseInput.xPos - 350.0f < 87.5f and g_mouseInput.xPos - 350.0f > 0.0f)//B
					{
						static_BoardSquare[rememberAttackedSquareForUserPromotion] = 18;
						WaitingForUserPromotion = false;
						MoveNum++;
					}
				}
			}
			else//black promotes
			{
				if (-g_mouseInput.yPos + 360.0f <= -87.5f and -g_mouseInput.yPos + 360.0f >= -175.0f)//the 'bottom' ones
				{
					if (g_mouseInput.xPos - 350.0f < 0.0f and g_mouseInput.xPos - 350.0f > -87.5f)//q
					{
						static_BoardSquare[rememberAttackedSquareForUserPromotion] = 13;
						WaitingForUserPromotion = false;
						MoveNum++;
					}
					else if (g_mouseInput.xPos - 350.0f < 87.5f and g_mouseInput.xPos - 350.0f > 0.0f)//r
					{
						static_BoardSquare[rememberAttackedSquareForUserPromotion] = 12;
						WaitingForUserPromotion = false;
						MoveNum++;
					}
				}
				else if (-g_mouseInput.yPos + 360.0f <= 0.0f and -g_mouseInput.yPos + 360.0f >= -87.5f)//the 'top' ones
				{
					if (g_mouseInput.xPos - 350.0f < 0.0f and g_mouseInput.xPos - 350.0f > -87.5f)//n
					{
						static_BoardSquare[rememberAttackedSquareForUserPromotion] = 11;
						WaitingForUserPromotion = false;
						MoveNum++;
					}
					else if (g_mouseInput.xPos - 350.0f < 87.5f and g_mouseInput.xPos - 350.0f > 0.0f)//b
					{
						static_BoardSquare[rememberAttackedSquareForUserPromotion] = 10;
						WaitingForUserPromotion = false;
						MoveNum++;
					}
				}
			}
		}

	}


	return quads;
}

//could output a vector, but this is chess, not a game engine
std::array<VertexStructure, 540> RenderChessPieces::MemcopyObjects(std::array<std::array<VertexStructure, 4Ui64>,135>& quads)
{

	std::array<VertexStructure, 540> positions{};
	for (int i = 0; i < 135; i++)
	{
		int PositionCount = i * 4;
		positions[PositionCount].Position = quads[i][0].Position;
		positions[PositionCount].TexCoords = quads[i][0].TexCoords;
		positions[PositionCount].TexID = quads[i][0].TexID;
		positions[PositionCount+1].Position = quads[i][1].Position;
		positions[PositionCount+1].TexCoords = quads[i][1].TexCoords;
		positions[PositionCount+1].TexID = quads[i][1].TexID;
		positions[PositionCount+2].Position = quads[i][2].Position;
		positions[PositionCount+2].TexCoords = quads[i][2].TexCoords;
		positions[PositionCount+2].TexID = quads[i][2].TexID;
		positions[PositionCount+3].Position = quads[i][3].Position;
		positions[PositionCount+3].TexCoords = quads[i][3].TexCoords;
		positions[PositionCount+3].TexID = quads[i][3].TexID;
	}
	
	return positions;
}

void RenderChessPieces::WillCanCastleChange(unsigned int PieceType, unsigned int BoardSquareNumItMovedFrom, const unsigned int& BoardSquareItMovedTo, canCastle& Castle)
{
	if (BoardSquareNumItMovedFrom == 4 and PieceType == 22)
	{
		Castle.HasWhiteKingMoved = true;
		if (BoardSquareItMovedTo == 6)
		{
			Castle.HasWhiteShortRookMoved = true;
		}
		else if (BoardSquareItMovedTo == 2)
		{
			Castle.HasWhiteLongRookMoved = true;
		}
	}
	else if (PieceType == 14 and BoardSquareNumItMovedFrom == 60)
	{
		Castle.HasBlackKingMoved = true;
		if (BoardSquareItMovedTo == 62)
		{
			Castle.HasBlackShortRookMoved = true;
		}
		else if (BoardSquareItMovedTo == 58)
		{
			Castle.HasBlackLongRookMoved = true;
		}
	}
	else if (PieceType == 20 and BoardSquareNumItMovedFrom == 0)
		Castle.HasWhiteLongRookMoved = true;
	else if (PieceType == 20 and BoardSquareNumItMovedFrom == 7)
		Castle.HasWhiteShortRookMoved = true;
	else if (PieceType == 12 and BoardSquareNumItMovedFrom == 63)
		Castle.HasBlackShortRookMoved = true;
	else if (PieceType == 12 and BoardSquareNumItMovedFrom == 56)
		Castle.HasBlackLongRookMoved = true;
}

float RenderChessPieces::GetPieceTextureID(std::array<unsigned int, 64> BoardSquare, unsigned int i)
{
	
	switch (BoardSquare[i]) 
	{
		case 0:
			return 13.0f;//none
		case 9:
			return 1.0f;//p
		case 10:
			return 2.0f;//b
		case 11:
			return 3.0f;//n
		case 12:
			return 4.0f;//r
		case 13:
			return 5.0f;//q
		case 14:
			return 6.0f;//k
		case 17:
			return 7.0f;//P
		case 18:
			return 8.0f;//B
		case 19:
			return 9.0f;//N
		case 20:
			return 10.0f;//R
		case 21:
			return 11.0f;//Q
		case 22:
			return 12.0f;//K
		default:
			ASSERT(false);
			return -1;
	}
}

uint8_t RenderChessPieces::GetPieceTypefromTexID(float TexID)
{
	if (TexID == 13.0f)
	{
		return 0;//none
	}
	else if (TexID == 1.0f) { return 9; }//p
	else if (TexID == 2.0f) { return 10; }//b
	else if (TexID == 3.0f) { return 11; }//n
	else if (TexID == 4.0f) { return 12; }//r
	else if (TexID == 5.0f) { return 13; }
	else if (TexID == 6.0f) { return 14; }
	else if (TexID == 7.0f) { return 17; }
	else if (TexID == 8.0f) { return 18; }
	else if (TexID == 9.0f) { return 19; }
	else if (TexID == 10.0f) { return 20; }
	else if (TexID == 11.0f) { return 21; }
	else if (TexID == 12.0f) { return 22; }
	else
	{
		ASSERT(false);
		return -1;
	}
}

void RenderChessPieces::WasLeftButtonPressed()
{
	if (g_mouseInput.LeftButtonPressed == true)
	{
		g_mouseInput.WasLeftButtonPressed = true;
	}
	if (g_mouseInput.LeftButtonPressed == false)
		g_mouseInput.WasLeftButtonPressed = false;
}

void RenderChessPieces::GetMouseInput(GLFWwindow* window)
{
	glfwSetCursorPosCallback(window, cursorPositionCallBack);
	glfwSetMouseButtonCallback(window, mouseButtonCallBack);
	glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, 1);
	//std::cout << g_mouseInput.xPos << ":" << g_mouseInput.yPos << '\n';
	//std::cout << "mouseLeft pressed = " << g_mouseInput.LeftButtonPressed << '\n';
	//std::cout << "was Left pressed = " << g_mouseInput.WasLeftButtonPressed << '\n';
 
}

void RenderChessPieces::SetStaticBoardSquare(const std::array<unsigned int, 64>& BoardSquare)
{
	if (wasStatic_BoardSquareCreated == false)
	{
		static_BoardSquare = BoardSquare;
		wasStatic_BoardSquareCreated = true;
	}
}

uint32_t RenderChessPieces::Perft(std::array<unsigned int, 64Ui64> BoardSquare, std::array<unsigned int, 64> perftPreviousBoardSquare, canCastle CanCastle, bool isNextMoveForWhite, uint8_t depth, bool DivideFunON, unsigned int& PerftMoveNum)
{
	uint32_t NumOfMoves = 0;
	//LOOK AT THIS DUMBASS LOOK AT THIS DUMBASS LOOK AT THIS DUMBASS LOOK AT THIS DUMBASS 
	//after a1b1 e8f8 e5f7 
	//f6 is considered pinned but it's not, WhichBoardSquaresAreAbsPinned
	//LOOK AT THIS DUMBASS LOOK AT THIS DUMBASS LOOK AT THIS DUMBASS LOOK AT THIS DUMBASS 
	GenerateLegalMoves LegalMoves(BoardSquare, &perftPreviousBoardSquare, CanCastle, isNextMoveForWhite, MoveNum, false);
	//LOOK AT THIS DUMBASS LOOK AT THIS DUMBASS LOOK AT THIS DUMBASS LOOK AT THIS DUMBASS
	//LOOK AT THIS DUMBASS LOOK AT THIS DUMBASS LOOK AT THIS DUMBASS LOOK AT THIS DUMBASS
	//LOOK AT THIS DUMBASS LOOK AT THIS DUMBASS LOOK AT THIS DUMBASS LOOK AT THIS DUMBASS
	//LOOK AT THIS DUMBASS LOOK AT THIS DUMBASS LOOK AT THIS DUMBASS LOOK AT THIS DUMBASS
	const auto ConstBoardSquare = BoardSquare;
	const auto ConstPreviousBoardSquare = perftPreviousBoardSquare;
	const canCastle ConstCanCastle = CanCastle;
	const auto ConstMoveNum = MoveNum;
	unsigned int count = 0;

	for (MOVE piece : LegalMoves.moves)
	{
		for (uint8_t move : piece.TargetSquares)
		{
			//perft the promotions, this if is basically a blunt .find()
			if (piece.Promotion[0] != 65 and piece.Promotion[0] == move or piece.Promotion[1] != 65 and piece.Promotion[1] == move or piece.Promotion[2] != 65 and piece.Promotion[2] == move)
			{
				bool IsWhite = Board::IsPieceColorWhite(BoardSquare[count]);

				for (uint8_t i = 0; i != 4; ++i)
				{
					if (depth == 1)
					{
						if (DivideFunON)
						{
							if (IsWhite)
								std::cout << count << " " << +move << " to " << Board::PieceType2letter(i + 18) << ": 1" << '\n';
							else
								std::cout << count << " " << +move << " to " << Board::PieceType2letter(i + 10) << ": 1" << '\n';
						}
						NumOfMoves++;
					}
					else
					{

						if (IsWhite)
							MakeMove(count, move, PerftMoveNum, BoardSquare, perftPreviousBoardSquare, CanCastle, i + 18);
						else
							MakeMove(count, move, PerftMoveNum, BoardSquare, perftPreviousBoardSquare, CanCastle, i + 10);
						if (DivideFunON)
						{
							uint32_t DivideFunNum = 0;
							DivideFunNum += Perft(BoardSquare, perftPreviousBoardSquare, CanCastle, !isNextMoveForWhite, depth - 1, false, PerftMoveNum);
							NumOfMoves += DivideFunNum;
							if (IsWhite)
								std::cout << count << " " << +move << ": " << DivideFunNum << " to " << Board::PieceType2letter(i + 18) << '\n';
							else
								std::cout << count << " " << +move << ": " << DivideFunNum << " to " << Board::PieceType2letter(i + 10) << '\n';
						}
						else
							NumOfMoves += Perft(BoardSquare, perftPreviousBoardSquare, CanCastle, !isNextMoveForWhite, depth - 1, false, PerftMoveNum);

						BoardSquare = perftPreviousBoardSquare;
					}
				}

				if (depth != 1)
				{
					BoardSquare = ConstBoardSquare;
					perftPreviousBoardSquare = ConstPreviousBoardSquare;
					CanCastle = ConstCanCastle;
					MoveNum = ConstMoveNum;
				}

				if (IsWhite)
					piece.Promotion[move - count - 7] = 65;
				else
					piece.Promotion[move - count + 9] = 65;

				continue;
			}

			if (depth == 1)
			{
				if(DivideFunON)
					std::cout << count << " " << +move << ": 1" << '\n';
				NumOfMoves++;
			}
			else
			{
				MakeMove(count, move, PerftMoveNum, BoardSquare, perftPreviousBoardSquare, CanCastle, 65);
				if (DivideFunON)
				{
					uint32_t DivideFunNum = 0;
					DivideFunNum += Perft(BoardSquare, perftPreviousBoardSquare, CanCastle, !isNextMoveForWhite, depth - 1, false, PerftMoveNum);
					NumOfMoves += DivideFunNum;
					std::cout << count << " " << +move << ": " << DivideFunNum << '\n';
				}
				else
					NumOfMoves += Perft(BoardSquare, perftPreviousBoardSquare, CanCastle, !isNextMoveForWhite, depth - 1, false, PerftMoveNum);

				BoardSquare = ConstBoardSquare;
				perftPreviousBoardSquare = ConstPreviousBoardSquare;
				CanCastle = ConstCanCastle;
				MoveNum = ConstMoveNum;
			}
		}
		count++;
	}

	return NumOfMoves;
}

void RenderChessPieces::MakeMove(unsigned int BoardSquare, unsigned int move, unsigned int& PerftMoveNum, std::array<unsigned int, 64>& fun_BoardSquare, std::array<unsigned int, 64>& fun_previousBoardSquare, canCastle& Castle, const uint8_t& PieceTypeToPromoteTo)
{
	fun_previousBoardSquare = fun_BoardSquare;
	WillCanCastleChange(fun_BoardSquare[BoardSquare], BoardSquare, move, Castle);
	fun_BoardSquare[move] = fun_BoardSquare[BoardSquare];

	//castling
	if (fun_BoardSquare[BoardSquare] == 22 or fun_BoardSquare[BoardSquare] == 14)
	{
		if (BoardSquare - move == -2)
		{
			if (BoardSquare == 4)
			{
				fun_BoardSquare[5] = 20;
				fun_BoardSquare[7] = 0;
			}
			else
			{
				fun_BoardSquare[61] = 12;
				fun_BoardSquare[63] = 0;
			}
		}
		if (BoardSquare - move == 2)
		{
			if (BoardSquare == 4)
			{
				fun_BoardSquare[3] = 20;
				fun_BoardSquare[0] = 0;
			}
			else
			{
				fun_BoardSquare[59] = 12;
				fun_BoardSquare[56] = 0;
			}
		}

	}

	//promoting
	if (PieceTypeToPromoteTo != 65)
	{
		fun_BoardSquare[move] = PieceTypeToPromoteTo;
	}
	else//optimization
	{
		//white en passant
		if (fun_BoardSquare[BoardSquare] == 17)
		{
			if (fun_previousBoardSquare[move] == 0)
			{
				if (BoardSquare - move == -7 or BoardSquare - move == -9)
				{
					fun_BoardSquare[move - 8] = 0;
				}
			}
		}
		//black en passant
		if (fun_BoardSquare[BoardSquare] == 9)
		{
			if (fun_previousBoardSquare[move] == 0)
			{
				if (BoardSquare - move == 7 or BoardSquare - move == 9)
				{
					fun_BoardSquare[move + 8] = 0;
				}
			}
		}
	}

	fun_BoardSquare[BoardSquare] = 0;
	
	PerftMoveNum++;
}

void RenderChessPieces::CreatePerft(uint8_t PerftDepth)
{
	//std::cout << "Worker at start CPU: " << GetCurrentProcessorNumber() << std::endl;
	canCastle perftCastle = CanCastle;
	auto perftBoardsquare = static_BoardSquare;
	auto perftPreviousBoardsquare = previousBoardsquare;
	auto Movenum = MoveNum;

	bool isNextMoveForWhite = true;
	if (Movenum % 2 != 0)
		isNextMoveForWhite = false;

	auto start = std::chrono::high_resolution_clock::now();
	std::cout << "Nodes searched: " << Perft(perftBoardsquare, perftPreviousBoardsquare, perftCastle, isNextMoveForWhite, PerftDepth, true, Movenum) << '\n';
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	std::cout << "In " << duration.count() << " ms" << '\n' << std::endl;

	//std::cout << "Worker CPU: " << GetCurrentProcessorNumber() << std::endl;
}

void RenderChessPieces::SetMoveNum(const uint32_t& Movenum)
{
	MoveNum = Movenum;
}

void RenderChessPieces::SetCanCastle(const canCastle& f_canCastle)
{
	CanCastle = f_canCastle;
}

void RenderChessPieces::SetPrevBoardSquare_FEN_EP(const uint32_t& f_BoardSquare)
{
	if (f_BoardSquare == 65)
		return;
	uint32_t f_previousBoardSquare = 0;
	wasStatic_previousBoardsquareCreated = true;
	if(f_BoardSquare < 32)
		f_previousBoardSquare = f_BoardSquare - 16;
	else
		f_previousBoardSquare = f_BoardSquare + 16;
	auto BS_copy = static_BoardSquare;
	auto f_PieceType = BS_copy[f_BoardSquare];
	BS_copy[f_BoardSquare] = 0;
	BS_copy[f_previousBoardSquare] = f_PieceType;
	previousBoardsquare = BS_copy;
}
