void MainMenu()
{
  motor.stop_all();
  LCD.clear(); LCD.home();
  LCD.print("Entering Main Menu");
  delay(500);
  LCD.clear();
  int menuIndex = countMainMenu - knob(6) * (countMainMenu) / 1024 - 1;
  int topIndex;
  if (menuIndex < countMainMenu - 1) {
    topIndex = menuIndex;
  } else {
    topIndex = menuIndex - 1;
  }
  int cursorPosition = menuIndex - topIndex;
  LCD.print(mainMenuNames[topIndex]); LCD.setCursor(0, 1); LCD.print(mainMenuNames[topIndex + 1]);
  LCD.setCursor(0, cursorPosition);
  LCD.cursor();
  while (true)
  {
    menuIndex = countMainMenu - knob(6) * (countMainMenu) / 1024 - 1;
    if (cursorPosition != menuIndex - topIndex)
    {
      LCD.clear();
      if (menuIndex > topIndex + 1) {
        topIndex++;
      }
      if (menuIndex < topIndex) {
        topIndex--;
      }
      cursorPosition = menuIndex - topIndex;
      LCD.print(mainMenuNames[topIndex]); LCD.setCursor(0, 1); LCD.print(mainMenuNames[topIndex + 1]);
      LCD.setCursor(0, cursorPosition);
    }
    delay(100);
    if (startbutton())
    {
      delay(100);
      if (startbutton()) {
        LCD.noCursor();
        (*menuFunctions[menuIndex])();
        LCD.clear();
        LCD.print(mainMenuNames[topIndex]); LCD.setCursor(0, 1); LCD.print(mainMenuNames[topIndex + 1]);
        LCD.setCursor(0, cursorPosition);
        LCD.cursor();
      }
    }
    if (stopbutton())
    {
      delay(100);
      if (stopbutton())
      {
        LCD.clear(); LCD.home(); LCD.noCursor();
        LCD.print("Leaving menu");
        delay(500);
        return;
      }
    }
  }
}

void Menu()
{
  LCD.clear(); LCD.home();
  LCD.print("Entering menu");
  delay(500);

  while (true)
  {
    /* Show MenuItem value and knob value */
    int menuIndex = knob(6) * (MenuItem::MenuItemCount) / 1024;
    LCD.clear(); LCD.home();
    LCD.print(menuItems[menuIndex].Name); LCD.print(" "); LCD.print(menuItems[menuIndex].Value);
    LCD.setCursor(0, 1);
    LCD.print("Set to "); LCD.print(knob(7) / divisors[menuIndex]); LCD.print("?");
    delay(100);

    /* Press start button to save the new value */
    if (startbutton())
    {
      delay(200);
      if (startbutton())
      {
        menuItems[menuIndex].Value = knob(7) / divisors[menuIndex];
        menuItems[menuIndex].Save();
        delay(250);
      }
    }

    /* Press stop button to exit menu */
    if (stopbutton())
    {
      delay(100);
      if (stopbutton())
      {
        LCD.clear(); LCD.home();
        LCD.print("Leaving menu");
        g = menuItems[0].Value;
        kp = menuItems[1].Value;
        kd = menuItems[2].Value;
        vel = menuItems[3].Value;
        intGain = menuItems[4].Value;
        delay(500);

        return;
      }
    }
  }
}

void ViewDigital()
{
  LCD.clear(); LCD.print("Viewing Digital");
  delay(500);
  LCD.clear();
  while (true)
  {
    int index = knob(6) * 3 / 1024;
    if (index == 0)
    {
      LCD.print("D0:"); LCD.print(digitalRead(0)); LCD.print(" D1:"); LCD.print(digitalRead(1)); LCD.print(" D2:"); LCD.print(digitalRead(2));
      LCD.setCursor(0, 1);
      LCD.print("D3:"); LCD.print(digitalRead(3)); LCD.print(" D4:"); LCD.print(digitalRead(4)); LCD.print(" D5:"); LCD.print(digitalRead(5));


    } else if (index == 1)
    {
      LCD.print("D6:"); LCD.print(digitalRead(6)); LCD.print(" D7:"); LCD.print(digitalRead(7)); LCD.print(" D8:"); LCD.print(digitalRead(8));
      LCD.setCursor(0, 1);
      LCD.print("D9:"); LCD.print(digitalRead(9)); LCD.print(" D0:"); LCD.print(digitalRead(10)); LCD.print(" D1:"); LCD.print(digitalRead(11));
    } else if (index == 2)
    {
      LCD.print("D2:"); LCD.print(digitalRead(12)); LCD.print(" D3:"); LCD.print(digitalRead(13)); LCD.print(" D4:"); LCD.print(digitalRead(14));
      LCD.setCursor(0, 1);
      LCD.print("D5:"); LCD.print(digitalRead(15));
    }

    delay(100);
    LCD.clear();
    if (stopbutton())
    {
      LCD.clear(); LCD.home();
      LCD.print("Leaving menu");
      delay(500);
      return;
    }
  }
}

void ViewAnalog()
{
  LCD.clear(); LCD.print("Viewing Analog");
  delay(500);
  LCD.clear();
  while (true)
  {
    int index = knob(6) * 2 / 1024;
    if (index == 0)
    {
      LCD.print("A0:"); LCD.print(analogRead(0)); LCD.setCursor(8, 0); LCD.print("A1:"); LCD.print(analogRead(1));
      LCD.setCursor(0, 1);
      LCD.print("A2:"); LCD.print(analogRead(2)); LCD.setCursor(8, 1); LCD.print("A3:"); LCD.print(analogRead(3));
    } else if (index == 1)
    {
      LCD.print("A4:"); LCD.print(analogRead(4)); LCD.setCursor(8, 0); LCD.print("A5:"); LCD.print(analogRead(5));
      LCD.setCursor(0, 1);
      LCD.print("A6:"); LCD.print(analogRead(6)); LCD.setCursor(8, 1); LCD.print("A7:"); LCD.print(analogRead(7)) ;
    }

    delay(100);
    LCD.clear();
    if (stopbutton())
    {
      LCD.clear(); LCD.home();
      LCD.print("Leaving menu");
      delay(500);
      return;
    }
  }
}

void ControlArm()
{
  int armAngle = armHome;
  int clawAngle = 0;
  int spd = 0;
  int count = 0;
  LCD.clear(); LCD.print("Controlling Arm");
  delay(500);
  LCD.clear();
  LCD.print("Press Start to begin");
  while (true) {
    if (startbutton()) {
      break;
    }
  }
  while (true) {
    clawAngle = (int)(knob(7) * 110.0 / 1024.0);
    armAngle = (int)(knob(6) * 190.0 / 1024.0);
    spd =  -255 + (int)(knob(6) * 512.0 / 1024.0);

    RCServo0.write(clawAngle);

    if (!startbutton() && !stopbutton()) {
      motor.speed(2, 0);
      RCServo1.write((int)(armAngle));
    }

    else if (stopbutton()) {
      motor.speed(2, spd);
    }

    if (count == 2000) {
      count = 0;
      LCD.clear();
      LCD.print("Claw: "); LCD.print(clawAngle);
      LCD.setCursor(0, 1);
      LCD.print("Arm: "); LCD.print(armAngle); LCD.print(" Spd: "); LCD.print(spd);
    }


    if (stopbutton() && !startbutton())
    {
      RCServo0.write(clawHome);
      RCServo1.write(armHome);
      LCD.clear(); LCD.home();
      LCD.print("Leaving Arm Control");
      delay(50);
      return;
    }
    count++;
  }
}

void PickupPassengerMain(){
  LCD.clear(); LCD.print("Press start");
  LCD.setCursor(0,1); LCD.print("Pickup on Right");
  delay(500);
    
  while(true){
    if(startbutton()){
      break;
    }
    delay(50);
  }
  PickupPassenger(-1);
  
  LCD.clear(); LCD.print("Press start");
  LCD.setCursor(0,1); LCD.print("Pickup on Left");
  delay(500);
    
  while(true){
    if(startbutton()){
      break;
    }
    delay(50);
  }
  PickupPassenger(1);
}

void altMotor(void){
  int i = 1;
  while(true){
    motor.speed(1,200*i);
    delay(2000);
    i = i*-1;
  }
}

