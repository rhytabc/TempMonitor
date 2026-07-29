package com.tempmonitor;

import android.app.Activity;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.ToggleButton;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

public class MainActivity extends Activity {

    private static final String ESP32_URL = "http://192.168.4.1/temp";
    private static final int POLL_MS = 1500;
    private static final float AIR_LOW=0,AIR_HIGH=35,WATER_LOW=5,WATER_HIGH=45;

    private float tCur=0,tMax=-99,tMin=99;
    private boolean hasData=false;
    private TempDescHelper.Mode mode=TempDescHelper.Mode.AIR;
    private String lastUpd="--:--";

    private TextView tempText,tempUnit,statusText,descText,maxText,minText,updateText;
    private TextView alarmLowText,alarmHighText;
    private LinearLayout statusBar;
    private ToggleButton modeBtn;
    private ChartView chartView;
    private TextView modeLabel;

    private TempService tempService;
    private Handler handler;
    private boolean running=false;

    @Override protected void onCreate(Bundle s){
        super.onCreate(s);
        setContentView(R.layout.activity_main);

        tempText=findViewById(R.id.temp_text); tempUnit=findViewById(R.id.temp_unit);
        statusText=findViewById(R.id.status_text); descText=findViewById(R.id.desc_text);
        maxText=findViewById(R.id.max_text); minText=findViewById(R.id.min_text);
        updateText=findViewById(R.id.update_text); statusBar=findViewById(R.id.status_bar);
        modeBtn=findViewById(R.id.mode_btn); chartView=findViewById(R.id.chart_view);
        modeLabel=findViewById(R.id.mode_label);
        alarmLowText=findViewById(R.id.alarm_low);
        alarmHighText=findViewById(R.id.alarm_high);

        handler=new Handler(Looper.getMainLooper());
        tempService=new TempService();

        updateAlarmLabels();
        modeBtn.setOnCheckedChangeListener((btn,checked)->{
            mode=checked?TempDescHelper.Mode.WATER:TempDescHelper.Mode.AIR;
            modeLabel.setText(checked?"WATER":"AIR");
            updateAlarmLabels();
            refreshDisplay();
        });
    }

    private void updateAlarmLabels(){
        if(mode==TempDescHelper.Mode.AIR){alarmLowText.setText("< 0°C");alarmHighText.setText("> 35°C");}
        else {alarmLowText.setText("< 5°C");alarmHighText.setText("> 45°C");}
    }

    @Override protected void onResume(){running=true;startPoll();}
    @Override protected void onPause(){running=false;}

    private void startPoll(){
        new Thread(()->{
            while(running){
                TempService.Result r=tempService.fetchTemperature(ESP32_URL);
                handler.post(()->{if(r.success)onNewTemp(r.temperature);else setDisc(r.error);});
                try{Thread.sleep(POLL_MS);}catch(InterruptedException e){break;}
            }
        }).start();
    }

    private void onNewTemp(float t){
        if(!hasData){tMax=t;tMin=t;hasData=true;}
        else{if(t>tMax)tMax=t;if(t<tMin)tMin=t;}
        tCur=t;
        lastUpd=new SimpleDateFormat("HH:mm:ss",Locale.getDefault()).format(new Date());
        chartView.addPoint(t);
        refreshDisplay();
    }

    private void refreshDisplay(){
        float lo,hi;String type;
        if(mode==TempDescHelper.Mode.AIR){lo=AIR_LOW;hi=AIR_HIGH;type="气温";}
        else{lo=WATER_LOW;hi=WATER_HIGH;type="水温";}
        boolean alarm=(tCur<lo||tCur>hi);

        int color;String desc;
        if(alarm){
            color=Color.parseColor("#E74C3C");
            if(tCur<lo)desc="⚠ "+type+"过低! ("+lo+"~"+hi+"°C)";
            else desc="⚠ "+type+"过高! ("+lo+"~"+hi+"°C)";
        }else{
            color=TempDescHelper.getColor(tCur,mode);
            desc=TempDescHelper.getDescription(tCur,mode);
        }

        tempText.setText(String.format(Locale.getDefault(),"%.1f",tCur));
        tempText.setTextColor(color);tempUnit.setTextColor(color);
        descText.setText(desc);
        maxText.setText(String.format(Locale.getDefault(),"%.1f°",tMax));
        minText.setText(String.format(Locale.getDefault(),"%.1f°",tMin));
        updateText.setText("Updated "+lastUpd);

        statusBar.setBackgroundColor(Color.parseColor("#FFFFFF"));
        statusText.setText("● Connected");statusText.setTextColor(Color.parseColor("#4CAF50"));
    }

    private void setDisc(String e){
        statusBar.setBackgroundColor(Color.parseColor("#DDDDDD"));
        statusText.setText("○ "+e);statusText.setTextColor(Color.parseColor("#999999"));
    }
}
