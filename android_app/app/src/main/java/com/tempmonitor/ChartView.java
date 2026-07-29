package com.tempmonitor;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.util.AttributeSet;
import android.view.View;

import java.util.LinkedList;

/**
 * 简易温度曲线图控件 — 显示最近的数据趋势
 */
public class ChartView extends View {

    private static final int MAX_POINTS = 60; // 最多 60 个数据点
    private static final int LINE_COLOR = 0xFFE94560;
    private static final int FILL_COLOR  = 0x20E94560;
    private static final int GRID_COLOR  = 0xFFF0F0F0;
    private static final int TEXT_COLOR  = 0xFF999999;

    private final LinkedList<Float> data = new LinkedList<>();
    private final Paint linePaint, fillPaint, gridPaint, textPaint;
    private float minVal = 0, maxVal = 50;

    public ChartView(Context context, AttributeSet attrs) {
        super(context, attrs);

        linePaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        linePaint.setStyle(Paint.Style.STROKE);
        linePaint.setStrokeWidth(4f);
        linePaint.setColor(LINE_COLOR);
        linePaint.setStrokeCap(Paint.Cap.ROUND);
        linePaint.setStrokeJoin(Paint.Join.ROUND);

        fillPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        fillPaint.setStyle(Paint.Style.FILL);
        fillPaint.setColor(FILL_COLOR);

        gridPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        gridPaint.setStyle(Paint.Style.STROKE);
        gridPaint.setStrokeWidth(1f);
        gridPaint.setColor(GRID_COLOR);

        textPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        textPaint.setTextSize(24f);
        textPaint.setColor(TEXT_COLOR);
    }

    /**
     * 添加一个新数据点
     */
    public void addPoint(float value) {
        data.add(value);
        if (data.size() > MAX_POINTS) data.removeFirst();

        // 动态调整 Y 轴范围
        float dMin = value, dMax = value;
        for (float v : data) {
            if (v < dMin) dMin = v;
            if (v > dMax) dMax = v;
        }
        // 给上下留点余量
        float range = dMax - dMin;
        if (range < 5) range = 5;
        minVal = dMin - range * 0.2f;
        maxVal = dMax + range * 0.2f;

        invalidate();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        float w = getWidth();
        float h = getHeight();
        float padL = 48f, padR = 16f, padT = 24f, padB = 48f;
        float chartW = w - padL - padR;
        float chartH = h - padT - padB;

        if (data.isEmpty()) {
            canvas.drawText("-- Waiting for data --", padL + 20, h / 2, textPaint);
            return;
        }

        // 画网格
        for (int i = 0; i <= 4; i++) {
            float y = padT + chartH * i / 4f;
            canvas.drawLine(padL, y, padL + chartW, y, gridPaint);
            float val = maxVal - (maxVal - minVal) * i / 4f;
            canvas.drawText(String.format("%.0f°", val), 0, y + 8, textPaint);
        }

        // 没有足够数据点就画网格
        if (data.size() < 2) return;

        // 画填充
        Path fillPath = new Path();
        float x0 = padL;
        float y0 = padT + chartH - ((data.get(0) - minVal) / (maxVal - minVal)) * chartH;
        fillPath.moveTo(x0, padT + chartH);
        fillPath.lineTo(x0, clampY(y0, padT, padT + chartH));

        for (int i = 1; i < data.size(); i++) {
            float xi = padL + ((float) i / (MAX_POINTS - 1)) * chartW;
            float yi = padT + chartH - ((data.get(i) - minVal) / (maxVal - minVal)) * chartH;
            yi = clampY(yi, padT, padT + chartH);
            fillPath.lineTo(xi, yi);
        }

        float lastX = padL + ((float) (data.size() - 1) / (MAX_POINTS - 1)) * chartW;
        fillPath.lineTo(lastX, padT + chartH);
        fillPath.close();
        canvas.drawPath(fillPath, fillPaint);

        // 画折线
        Path linePath = new Path();
        linePath.moveTo(x0, y0);
        for (int i = 1; i < data.size(); i++) {
            float xi = padL + ((float) i / (MAX_POINTS - 1)) * chartW;
            float yi = padT + chartH - ((data.get(i) - minVal) / (maxVal - minVal)) * chartH;
            yi = clampY(yi, padT, padT + chartH);
            linePath.lineTo(xi, yi);
        }
        canvas.drawPath(linePath, linePaint);
    }

    private float clampY(float y, float min, float max) {
        return Math.max(min, Math.min(max, y));
    }

    public void clear() {
        data.clear();
        invalidate();
    }
}
