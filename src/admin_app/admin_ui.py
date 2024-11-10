from tkinter import *
import tkinter as tk
import tkinter.ttk as ttk
from PIL import Image, ImageTk
import pandas as pd
from tkinter.filedialog import askopenfilename

import smtplib
from email.mime.base import MIMEBase
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email import encoders 
import json

# Add admin email and password
admin_usr_email = "example@gmail.com"
admin_usr_pw = "your_password"

# Read mail user
user_name = json.load(open('./user_mail.json', encoding="utf8"))

# Convert JSON to dict
user_dt = {}

for component in user_name:
    user_dt.update(component)

# Create Combobox
def Combobox(dispname, dictinput, x_place, y_place):
    key, value = list(dictinput.items())[0]

    option = list(dictinput.values()) 
    newlist = [" "]

    for i in option:
        for j in i:    
            newlist.append(j)

    variable = StringVar()

    tk.Label(root, text=dispname, background="#CEE3F6").place(x = x_place, y = y_place)                     
    menu = ttk.OptionMenu(root, variable, *newlist)        
    
    menu.config(width=25) 
    menu.place(x = x_place + 70, y = y_place)

    def check(*args):
        global user_name
        user_name = variable.get()
    
    variable.trace('w', check)


# Data Pre-Processing
class DataPreProcessing:
    def __init__(self):
        self.date_new = []
        self.time_new = []
        self.time_using = []
        self.status = []

    def Status(self):
        status = list(df.Status)
        for i in status:
            self.status.append(i)
        self.status = pd.Series(self.status)

    # Processing Date
    def Date(self):
        date = list(df.Date)
        # Split data
        for i in date:
            self.date_new.append(i.split('/')[1])
        self.date_new = pd.Series(self.date_new)
        # Replace data
        for i in range(len(df.Date)):
            df['Date'] = df['Date'].replace(df.Date[i],self.date_new[i])

    # Processing Time
    def Time(self):
        time = list(df.Time)
        # Split data
        for i in time:
            hh, mm , ss = map(int, i.split(':'))
            self.time_new.append(ss + 60*(mm + 60*hh))
        self.time_new = pd.Series(self.time_new)        
        # Replace data
        for i in range(len(df.Time)):
            df['Time'] = df['Time'].replace(df.Time[i], self.time_new[i])

    # Export to new CSV file
    def ExportNewData(self):
        df.to_csv("./testresult.csv")

class DataProcessing(DataPreProcessing):
    def __init__(self):
        super().__init__()
        self.time_using = []
        self.power = []
        self.totalpower = 0
        self.price = 0

    def CalculateTimeUsing(self):
        data = DataPreProcessing()
        data.Status()
        data.Date()
        data.Time()     

        for i in range(len(df.Date) - 1):
            # ON/OFF cùng 1 ngày
            if (data.date_new[i+1] == data.date_new[i]) and (data.status[i+1] == "OFF")\
                and (data.status[i] == "ON"):
                self.time_using.append(data.time_new[i+1] - data.time_new[i])
            # ON nhưng OFF ngày hôm sau  
            elif (data.date_new[i+1] > data.date_new[i]) and (data.status[i+1] == "OFF"):
                data.time_new[i+1] += 24*3600
                self.time_using.append(data.time_new[i+1] - data.time_new[i])
        print(self.time_using)

    def CalculatePower(self):
        for i in range(len(self.time_using)):
            self.power.append(self.time_using[i]*100/3600)
            self.totalpower += self.power[i]
        self.totalpower = round(self.totalpower/1000, 2)
        print(self.power)
        print(self.totalpower, "kWh")
    
    def CalculatePrice(self):
        self.price = self.totalpower * 3500
        print(self.price, "VNĐ")

def import_csv_data():
    global csvpath, df
    csv_file_path = askopenfilename()
    csvpath.set(csv_file_path)
    df = pd.read_csv(csv_file_path)

def sendmail():
    processing = DataProcessing()
    processing.CalculateTimeUsing()
    processing.CalculatePower()
    processing.CalculatePrice()

    fromaddr = admin_usr_email
    toaddr = user_name
    msg = MIMEMultipart()    
    msg['From'] = fromaddr
    msg['To'] = toaddr
    msg['Subject'] = "REPORT USING SMART HOME"
    body = f"Dear Mr/Mrs, \nThis is a report about using electrical of your family on this month: \n \
    - Total Power Using: {processing.totalpower} kWh \n \
    - Price: {processing.price} VNĐ"
            
    msg.attach(MIMEText(body, 'plain'))
    server = smtplib.SMTP('smtp.gmail.com', 587)
    server.starttls()
    server.login(fromaddr, admin_usr_pw)
    text = msg.as_string()
    server.sendmail(fromaddr, toaddr, text)
    server.quit()

# Create windows
root = tk.Tk()
root.geometry("500x500+300+300")
root.title('BKU Tool')
root.config(background="#CEE3F6")

# Insert Image
bard = Image.open("./BKTool.jpg")
bard = bard.resize((500, 100), Image.ANTIALIAS)
bardejov = ImageTk.PhotoImage(bard)
label1 = Label(root, image=bardejov)
label1.image = bardejov
label1.place(x = 0, y = 0)

# Create Combobox
combox1 = Combobox("Customer:", {"user_name": user_dt['user_name']}, 100, 220)

# Create Button, Text Box
ttk.Label(root, text='File Path:', background="#CEE3F6").place(x = 100, y = 170)
csvpath = tk.StringVar()
entry = ttk.Entry(root, textvariable = csvpath)
entry.place(x = 170, y = 170)
entry.config(width=30)
ttk.Button(root, text='Import CSV File',command=import_csv_data).place(x = 130, y = 330)

ttk.Button(root, text='Send Mail', command = sendmail).place(x = 280, y = 330)
root.mainloop() 