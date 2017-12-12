package FrendChat.Models;

import FrendChat.Presenters.Connect;
import javafx.concurrent.Task;
import java.io.IOException;
import java.net.Socket;

public class FrendServer {
    Socket socket;

    private static FrendServer ourInstance = new FrendServer();

    public static FrendServer getInstance() {
        return ourInstance;
    }

    private FrendServer() {
    }

    public void Connect(String ip, int port, Connect connect) {
        Task task = new Task<Void>() {
            @Override
            public Void call() {
                try {
                    socket = new Socket(ip, port);
                    connect.mdlConnectSuccessful();
                } catch (IOException e) {
                    connect.mdlConnectUnsuccessful();
                }
                return null;
            }
        };

        new Thread(task).start();

    }
}


//                    PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
//                    // Untested sample code
//                    //BufferedReader stdIn = new BufferedReader(new InputStreamReader(System.in));
//                    out.println("Hello World!");
//                    socket.close();