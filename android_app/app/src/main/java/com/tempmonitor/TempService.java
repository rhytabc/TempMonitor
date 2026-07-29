package com.tempmonitor;

import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;

/**
 * HTTP 温度数据服务
 *
 * 从 ESP32 网关 HTTP API 获取温度数据。
 * ESP32 API: GET /temp → {"temperature":25.3}
 */
public class TempService {

    private static final int CONNECT_TIMEOUT = 3000;
    private static final int READ_TIMEOUT    = 3000;

    /**
     * 单次 HTTP 查询结果
     */
    public static class Result {
        public boolean success;
        public float   temperature;
        public String  error;

        public Result(boolean success, float temperature, String error) {
            this.success     = success;
            this.temperature = temperature;
            this.error       = error;
        }
    }

    /**
     * 通过 HTTP GET 获取温度
     * @param url ESP32 的 API 地址，如 http://192.168.4.1/temp
     * @return 查询结果
     */
    public Result fetchTemperature(String urlString) {
        HttpURLConnection conn = null;
        try {
            URL url = new URL(urlString);
            conn = (HttpURLConnection) url.openConnection();
            conn.setConnectTimeout(CONNECT_TIMEOUT);
            conn.setReadTimeout(READ_TIMEOUT);
            conn.setRequestMethod("GET");

            int responseCode = conn.getResponseCode();
            if (responseCode != 200) {
                return new Result(false, 0, "HTTP " + responseCode);
            }

            /* 读取响应体 */
            BufferedReader reader = new BufferedReader(
                new InputStreamReader(conn.getInputStream()));
            StringBuilder sb = new StringBuilder();
            String line;
            while ((line = reader.readLine()) != null) {
                sb.append(line);
            }
            reader.close();

            /* 解析 JSON */
            JSONObject json = new JSONObject(sb.toString());
            if (json.isNull("temperature")) {
                return new Result(false, 0, "No sensor data");
            }

            float temp = (float) json.getDouble("temperature");
            return new Result(true, temp, null);

        } catch (java.net.SocketTimeoutException e) {
            return new Result(false, 0, "Timeout");
        } catch (java.net.ConnectException e) {
            return new Result(false, 0, "Not connected to WiFi");
        } catch (Exception e) {
            return new Result(false, 0, e.getMessage());
        } finally {
            if (conn != null) {
                conn.disconnect();
            }
        }
    }
}
