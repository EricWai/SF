import org.apache.http.client.methods.HttpPost;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;

import java.io.IOException;

public class Test {

    public static String url;

    static {
        url = System.getenv("sf-judge-server");
        if(url == null){
            url = "http://localhost:5555";
        }
    }

    public static void main(String[] args) throws IOException {

        CloseableHttpClient httpClient = HttpClients.createDefault();
        httpClient.execute(new HttpPost(url + "/finish_submission"));
    }

}