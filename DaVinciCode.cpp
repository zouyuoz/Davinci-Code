#include <iostream>
#include <cstdlib>
#include <cmath>//floor
#include <ctime>
#include <conio.h>
#include <windows.h>//setColor, Sleep

using namespace std;

/*
ABOUT THIS GAME
In this game, you need to play against three computer players
Guess the card of your oponents to get more clues
If you guess wrong, you reveal your card to your oponents,
make you more likely to loss!
Make a wise choice to chose a card to guess

HOW TO PLAY?
Upon enter the game, press ENTER to decide which player go first
The game will also ask you if you need hint
Hint will show you what card haven't been shown yet
If you drew dash card, you need to decide where do you want to put it (or them)
Otherwise just press enter to continue

When every run, that player will draw one card automatically
Then it will guess one oponent's card
If that player guess right, the oponent should turn the card so that everybody can see
If that player guess wrong, the player should turn the card which it just drawn

When guessing a player's card, you need to input three numbers
<player num> <card index> <number of that card>
Notice that card index start from zero
For example:
PLAYER0:  3  5  6  8  4 (this is you)
PLAYER1:  X  7  9 10
PLAYER2:  X  X  2      
PLAYER3:  X  X  X  X 11
If you think "player2's first card is 1",
then you should key in "2 0 1" and press enter
After that, system will tell you if you guess right or not,
and automatically determine which card should be shown

If card pool is empty, and one guess wrong,
system will ask you which card you want to shown
If one's cards are all shown, then it bye bye
The last one survive wins the game
*/

void setColor(float thatCard, bool showCard) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (int(thatCard * 10) % 5) thatCard = thatCard - 0.1;

	int det;
	if (thatCard != 99)det = int((thatCard - floor(thatCard)) * 2);
	else det = (int)thatCard;

	if (det == 0) {//red
		if (showCard) SetConsoleTextAttribute(hConsole, 207);
		else SetConsoleTextAttribute(hConsole, 72);
	}
	else if (det == 1) {//blue
		if (showCard) SetConsoleTextAttribute(hConsole, 159);
		else SetConsoleTextAttribute(hConsole, 24);
	}
	else {
		SetConsoleTextAttribute(hConsole, 7);
	}
}

struct playerData {
	float deckData[10] = {};
	bool showCard[10] = {};
	int gameOver = 0;
	float privateAnswer[26] = {};
};

struct guessedNumberRecord {
	float card[3] = { 99,99,99 };
	int player = 4;
	int position = 10;
	int times = 0;
}record;

//draw a card from the pool
float drawFromPool(float* poolLeft, int& cardLeft) {
	int rn = rand() % cardLeft;
	float chosenCard = poolLeft[rn];
	cardLeft--;
	for (int i = rn; i < cardLeft; i++)poolLeft[i] = poolLeft[i + 1];
	return chosenCard;
}

void outPlayerInfo(float* deckData, bool* showCard, float* privateAnswer, int& round, int who, bool hint, bool showAll = 0) {
	//out player: default color
	setColor(99, 0);
	if (privateAnswer[0] == 94) cout << " ";
	else cout << "PLAYER" << who << ": ";
	for (int j = 0; j < round; j++) {
		float thatCard = deckData[j];
		bool toShowCard = showCard[j];
		if (thatCard != 98 || floor(thatCard) == -1) {
			//determine color
			setColor(thatCard, toShowCard);

			if (toShowCard || !who || showAll) { // cheat: if (~~~ || 1)
				if (int(thatCard * 10) % 5 || floor(thatCard) == -1 || floor(thatCard) == 12) {
					cout << " -";
				}
				else {
					if (!(int(thatCard) / 10))cout << " ";
					cout << floor(thatCard);
				}
			}
			else cout << "  ";

			//reset color and give a space
			setColor(99, 0);
			cout << " ";
		}
	}
	if (hint && !who && privateAnswer[0] != 94) {
		cout << "|hint: ";
		for (int j = 0; j < 26; j++) {
			if (privateAnswer[j] == 99)continue;
			setColor(privateAnswer[j], 0);
			if (floor(privateAnswer[j]) == 12)cout << " -";
			else {
				if (!((int)privateAnswer[j] / 10))cout << " ";
				cout << floor(privateAnswer[j]);
			}
			setColor(99, 0);
			cout << " ";
		}
	}
	cout << endl << endl;
}

void computerGuessAlgorithm(int& playerToGuess, int& cardToGuess, float& thatCardIs, playerData* player, int thisComputer, int* round, guessedNumberRecord& record) {
	// scan all player's shown card (not himself and still alive)
	// find a smallest diff between shown cards, which is most possible to guess correctly
	// to do this, we need a loop to find a showCard == false
	// once found, write two while loop, find the prevShown and nextShown (if not, prev = 0; next = round[i] - 1)
	// and check float diffNextPre = player[i].deckData[next] - player[i].deckData[pre] (a.k.a. nextShown - prevShown)
	// find the smallest diffNextPre, if diffNextPre == 0.5: thatCardIs = -1;
	// else: rand a guess form privateAnswer, (guess >= prevShown && guess <= nextShown)

	float diffNextPrev = 99.0;
	float prevShown = 0, nextShown = 11.5;
	for (int i = 0; i < 4; i++) { // search all player
		if (i == thisComputer || player[i].gameOver) continue; // skip the for loop if guess himslef or the player died
		for (int j = 0; j < round[i]; j++) {
			if (!player[i].showCard[j]) { // found a unshown cared
				float tempPrevShown = 0, tempNextShown = 11.5;
				int prev = j, next = j;
				while (prev > 0 && !player[i].showCard[prev]) { // find prevShown
					prev--;
					if (player[i].showCard[prev]) {
						if (floor(player[i].deckData[prev]) == -1 || int(player[i].deckData[prev] * 10) % 5) continue;
						tempPrevShown = player[i].deckData[prev];
						break;
					}
				}
				while (next < round[i] && !player[i].showCard[next]) { // find nextShown
					next++;
					if (player[i].showCard[next]) {
						if (floor(player[i].deckData[next]) == -1 || int(player[i].deckData[next] * 10) % 5) continue;
						tempNextShown = player[i].deckData[next];
						break;
					}
				}
				if (tempNextShown - tempPrevShown < diffNextPrev) { // find the smallest diffNextPre
					// record the next & prevShown, player and cardToGuess
					nextShown = tempNextShown;
					prevShown = tempPrevShown;
					diffNextPrev = nextShown - prevShown;
					playerToGuess = i;
					cardToGuess = j;
				}
			}
		}
	}
	//check if guess the same player, same position. if not => reset all record
	if (playerToGuess != record.player || cardToGuess != record.position) {
		for (int i = 0; i < 3; i++) record.card[i] = 99;
		record.player = 4;
		record.position = 10;
		record.times = 0;
	}

	bool colorSame = 0;
	bool prevGuessedCard = 1; // prevGuessedRecord uses
	int scanThing = 0;
	do {
		thatCardIs = player[thisComputer].privateAnswer[scanThing];
		scanThing++;

		// check if guessed before, if in record.card => prevGuessCard = false
		prevGuessedCard = 1;
		if (playerToGuess == record.player && cardToGuess == record.position) {
			for (int i = 0; i < 3; i++) {
				if (record.card[i] == thatCardIs) prevGuessedCard = 0;
			}
		}

		// check if thatCardIs is the same color as the cardToGuess
		float detSum = thatCardIs + player[playerToGuess].deckData[cardToGuess];
		colorSame = !(int)((detSum - floor(detSum)) * 2); //colorSame, RHS = !0, colorDiff, RHS = !1

	} while (!(colorSame && thatCardIs != 99 && thatCardIs >= prevShown && thatCardIs <= nextShown && prevGuessedCard) && scanThing < 26);
	//colorSameAsThatCard && inPrivateAnswer && checkPreviousAndNext && checkGuessedBefore && findThrough
	if (scanThing == 26)thatCardIs = -1;

	record.card[record.times % 3] = thatCardIs;
	record.times++;
	record.player = playerToGuess;
	record.position = cardToGuess;
}

int ifDrawDash(float* deckData, bool* showCard, int length, int t_h = 99) {
	int head = t_h % 10;
	int tail = t_h / 10;
	cout << "\n\n\n";
	float noUse[1] = { 94 };
	int printLength = length - 1;
	outPlayerInfo(deckData, showCard, noUse, printLength, 0, 0, 0);

	HANDLE hHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	cout << "Choose where to insert\n(press enter after decided)";
	GetConsoleScreenBufferInfo(hHandle, &consoleInfo);
	COORD nowPos = consoleInfo.dwCursorPosition;
	int i = 0;
	COORD curPos;
	COORD numPos;
	curPos.Y = nowPos.Y - 4;
	numPos.Y = nowPos.Y - 5;
	if (t_h == 99) curPos.X = numPos.X = 0;
	else curPos.X = numPos.X = 3 * head;
	SetConsoleCursorPosition(hHandle, curPos);
	cout << "v";
	SetConsoleCursorPosition(hHandle, numPos);
	cout << curPos.X / 3;
	while (i != 13) {
		if (_kbhit()) {
			SetConsoleCursorPosition(hHandle, curPos);
			cout << " ";
			SetConsoleCursorPosition(hHandle, numPos);
			cout << " ";

			i = _getch();
			switch (i) { // _11_22_33_
			case 77:
				if ((t_h == 99 && curPos.X < 3 * printLength) || (t_h != 99 && curPos.X < 3 * tail)) {
					curPos.X += 3;
					numPos.X += 3;
				}
				break;
			case 75:
				if ((t_h == 99 && curPos.X > 0) || (t_h != 99 && curPos.X > 3 * head)) {
					curPos.X -= 3;
					numPos.X -= 3;
				}
				break;
			}
			SetConsoleCursorPosition(hHandle, curPos);
			cout << "v";
			SetConsoleCursorPosition(hHandle, numPos);
			cout << curPos.X / 3;
			GetConsoleScreenBufferInfo(hHandle, &consoleInfo);
			nowPos = consoleInfo.dwCursorPosition;
			nowPos.Y += 6;
			nowPos.X = 0;
			SetConsoleCursorPosition(hHandle, nowPos);
		}
	}
	int posit = curPos.X / 3;
	cout << "You chose " << posit << ", ";

	return posit;
}

void sortDeck(int who, float* deckData, bool* showCard, int howManyCards) {
	float key = deckData[howManyCards - 1];
	bool showKey = showCard[howManyCards - 1];
	int position = 0;

	if (floor(key) == 12) {
		float temp = key - floor(key); // record the color
		if (!who) { // change 12 to -1 (+ 0 or 0.5), and determine position using ifDrawDash
			deckData[howManyCards - 1] = -1 + temp;
			key = deckData[howManyCards - 1];
			if (howManyCards > 1) {
				cout << "You drew: ";
				setColor(key, showKey);
				cout << " -";
				setColor(99, 0);
				position = ifDrawDash(deckData, showCard, howManyCards);
			}
		}
		else { // rand a number and give it a 0.1
			deckData[howManyCards - 1] = rand() % 12 + temp + 0.1;
			key = deckData[howManyCards - 1];
			for (int i = 0; i < howManyCards; i++) {
				position = i;
				if (key < deckData[i]) break;
			}
		}
	}
	else {
		for (int i = 0; i < howManyCards; i++) {
			position = i;

			if (key < deckData[i]) break;
			else if (floor(deckData[i]) == -1) {
				if (deckData[i + 1] < key && (floor(deckData[i + 1]) != -1 || deckData[i + 2] < key)) continue;
				
				cout << "You drew: ";
				setColor(key, showKey);
				if (!(int(key) / 10))cout << " ";
				cout << floor(key);
				setColor(99, 0);

				int tail_head;
				if (floor(deckData[i + 1]) == -1)tail_head = (i + 2) * 10 + i;
				else tail_head = (i + 1) * 10 + i;

				position = ifDrawDash(deckData, showCard, howManyCards, tail_head);
				break;
			}
		}
	}
	for (int j = howManyCards - 1; j > position; j--) {
		deckData[j] = deckData[j - 1];
		showCard[j] = showCard[j - 1];
	}
	deckData[position] = key;
	showCard[position] = showKey;
}

void removePrivateAnswer(float* privateAnswer, float knownAnswer) {
	for (int j = 0; j < 26; j++) {
		if (privateAnswer[j] == knownAnswer || (floor(privateAnswer[j]) == 12 && int(knownAnswer * 10) % 5)) {
			privateAnswer[j] = 99;
			break;
		}
	}
}

void initialGame(playerData* player, float* poolLeft, int& cardLeft, int* round, int initialPlayer, int& hint) {
	system("cls");
	//start game
	char KEY = 0;
	cout << "====== WELCOME TO DAVINCI CODE! ======\n";
	cout << "PRESS ENTER TO CHOOSE THE FIRST PLAYER\n";
	cout << "+------------------------------------+\n";
	cout << "|            PLAYER# FIRST           |\n";
	cout << "+------------------------------------+";
	int changingNumber = 0;
	while (KEY != 13) {
		Sleep(80);
		if (_kbhit()) KEY = _getch();
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 19,3 });
		cout << changingNumber % 4;
		changingNumber++;
	}
	changingNumber = 12;
	for (; changingNumber; changingNumber--) {
		Sleep(960 / changingNumber);
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 19,3 });
		if (changingNumber == 1) SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10); // green
		cout << (changingNumber - 1 + initialPlayer) % 4;
	}
	setColor(99, 0);
	Sleep(360);
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 4,7 });
	cout << "DO YOU NEED HINTS (1 / 0): ";
	cin >> hint;
	cout << "    ";
	system("pause");
	system("cls");

	//generate pool, privateAnswer
	cardLeft = 26;
	for (int i = 0; i < 26; i++) {
		poolLeft[i] = (float)i / 2;
		for (int k = 0; k < 4; k++) player[k].privateAnswer[i] = (float)i / 2;
	}

	//for the zeroth round (round = 3)
	for (int i = 0; i < 4; i++) {
		round[i] = 0;
		for (int j = 0; j < 3; j++) {
			round[i]++;

			float theDrewCard = drawFromPool(poolLeft, cardLeft);
			player[i].deckData[j] = theDrewCard;
			removePrivateAnswer(player[i].privateAnswer, theDrewCard);
			player[i].showCard[j] = 0;

			//sort playerDeck
			sortDeck(i, player[i].deckData, player[i].showCard, round[i]);
		}
	}
	system("pause");
	system("cls");
}

int main() {
	time_t seed = 1686435675; //time(0), 1686409000(?), 1686467740, (goodExample)1686435675, 1686435676,
	//1686468467(doubleDash), (firstDash)1686420047, 1686425279, 
	srand(seed);

	//player data and initial game data
	playerData player[4];
	float poolLeft[26] = {};
	int cardLeft = 26;
	int checkDiffCardLeft = cardLeft;
	int round[4] = {};
	int initialPlayer = rand() % 4;
	int hint = 1;
	initialGame(player, poolLeft, cardLeft, round, initialPlayer, hint);


	//start game
	int over = 0;
	while (over != 3) {
		for (int i = 0; i < 4; i++) {
			//if the PLAYER_i game over, then skip this loop:
			if (player[i].gameOver) continue;
			if (initialPlayer) {
				initialPlayer--;
				continue;
			}
			//show whose turn
			cout << "\n+--------------------------------+";
			cout << "\n|         PLAYER" << i << "'s TURN         |seed: " << seed;
			cout << "\n+--------------------------------+\n";

			float theDrewCard;
			checkDiffCardLeft = cardLeft;
			if (cardLeft) { //do only when cardLeft
				round[i]++;
				theDrewCard = drawFromPool(poolLeft, cardLeft);
				player[i].deckData[round[i] - 1] = theDrewCard;
				player[i].showCard[round[i] - 1] = 0;
				removePrivateAnswer(player[i].privateAnswer, theDrewCard);
			}
			for (int i = 0; i < 4; i++) outPlayerInfo(player[i].deckData, player[i].showCard, player[i].privateAnswer, round[i], i, hint, 0);

			int playerToGuess, cardToGuess;
			float thatCardIs;
			if (!i) { //player
				do {
					cout << "Guess a PLAYER's card: ";
					cin >> playerToGuess >> cardToGuess >> thatCardIs;
					if (player[playerToGuess].showCard[cardToGuess])cout << "No! It has already shown!\n";
				} while (player[playerToGuess].showCard[cardToGuess]);
				if (playerToGuess == record.player && cardToGuess == record.position) {
					record.card[record.times % 3] = thatCardIs;
					record.times++;
					record.player = playerToGuess;
					record.position = cardToGuess;
				}
			}
			else { //computer
				//algorithm to get percise guess
				computerGuessAlgorithm(playerToGuess, cardToGuess, thatCardIs, player, i, round, record);
				Sleep(500);
			}

			cout << "Guess PLAYER" << playerToGuess << "'s " << cardToGuess << "th card is " << floor(thatCardIs);
			cout << " (" << playerToGuess << " " << cardToGuess << " " << floor(thatCardIs) << ")\n";
			Sleep(1000);

			//determine whether the guess is right
			bool preciseGuess = player[playerToGuess].deckData[cardToGuess] == thatCardIs;
			bool ignoreBlue = player[playerToGuess].deckData[cardToGuess] == thatCardIs + 0.5;
			bool guessDash = int(player[playerToGuess].deckData[cardToGuess] * 10) % 5 && thatCardIs == -1;
			if (preciseGuess || ignoreBlue || guessDash) {
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10); // green
				cout << "Congrats, you guess it right!\n";
				setColor(99, 0); // reset
				player[playerToGuess].showCard[cardToGuess] = true;
				for (int j = 0; j < 4; j++) removePrivateAnswer(player[j].privateAnswer, player[playerToGuess].deckData[cardToGuess]);
			}
			else {
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4); // red
				cout << "Oh no, you guess it wrong!\n";
				setColor(99, 0); // reset
				//cout << "it's actually " << player[playerToGuess].deckData[cardToGuess] << endl;
				if (cardLeft) {
					player[i].showCard[round[i] - 1] = true;
					for (int j = 0; j < 4; j++) removePrivateAnswer(player[j].privateAnswer, theDrewCard);
				}
				else {
					int theCardToShow = 10;
					if (!i) {
						do {
							cout << "Which card do you want to show: ";
							cin >> theCardToShow;
							if (player[i].showCard[theCardToShow])cout << "Error: it has already shown!\n";
							if (theCardToShow >= round[i])cout << "Error: not in deck data!\n";
						} while (player[i].showCard[theCardToShow] || theCardToShow >= round[i]);
					}
					else {
						do {
							theCardToShow = rand() % round[i];
						} while (player[i].showCard[theCardToShow]);
					}
					player[i].showCard[theCardToShow] = true;
					for (int j = 0; j < 4; j++) removePrivateAnswer(player[j].privateAnswer, player[i].deckData[theCardToShow]);
				}
			}
			//sort the deck
			if (checkDiffCardLeft - cardLeft) sortDeck(i, player[i].deckData, player[i].showCard, round[i]);

			//output game over info if a player died the first time
			for (int j = 0; j < 4; j++) {
				int detPlayerOver = 1;
				for (int k = 0; k < round[j]; k++) detPlayerOver *= player[j].showCard[k];
				if (detPlayerOver && !player[j].gameOver) {
					cout << "\n+---------------------------+";
					cout << "\n|       PLAYER" << j << " OVER!       |";
					cout << "\n+---------------------------+\n";
					player[j].gameOver = 1;
				}
			}

			//someone win and break the whole game
			over = 0;
			for (int i = 0; i < 4; i++)over += player[i].gameOver;
			if (over == 3) {
				for (int i = 0; i < 4; i++) {
					if (!player[i].gameOver) {
						cout << "\n+---------------------------------+";
						cout << "\n| * . * . *PLAYER" << i << " WINS!. * . * . |";
						cout << "\n+---------------------------------+\n";
						for (int i = 0; i < 4; i++) outPlayerInfo(player[i].deckData, player[i].showCard, player[i].privateAnswer, round[i], i, 0, 1);
					}
				}
				break;
			}
			system("pause");
			system("cls");
		}
	}
	return 0;
}
