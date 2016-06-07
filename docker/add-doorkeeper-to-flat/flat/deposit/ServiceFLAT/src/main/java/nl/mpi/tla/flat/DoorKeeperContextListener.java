/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package nl.mpi.tla.flat;

import java.util.LinkedHashMap;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import javax.servlet.ServletContextEvent;
import javax.servlet.ServletContextListener;
import nl.mpi.tla.flat.deposit.Flow;

/**
 *
 * @author menzowi
 */
public class DoorKeeperContextListener implements ServletContextListener {
    
    protected ExecutorService executor;
    protected LinkedHashMap<String,Flow> executed;
    
    public void contextInitialized(ServletContextEvent sce) {
        System.err.println("DoorKeeper: welcome!");
        String threads = sce.getServletContext().getInitParameter("doorkeeperThreads");
        executor = Executors.newFixedThreadPool(threads!=null?Integer.parseInt(threads):5);
        sce.getServletContext().setAttribute("DOORKEEPER",this);
        final String queue = sce.getServletContext().getInitParameter("doorkeeperQueue");
        executed = new LinkedHashMap<String,Flow>() {
            protected boolean removeEldestEntry(Map.Entry eldest) {
            return size() > (queue!=null?Integer.parseInt(queue):100);
        }};
    }
    
    public boolean execute(String sip,Flow flow) {
        synchronized(this) {
            if (executed.containsKey(sip)) {
                Boolean status = executed.get(sip).getStatus();
                if (status == null) {
                    System.err.println("DoorKeeper: sip["+sip+"] is already being executed!");
                    return false;
                }
            }
            executed.put(sip,flow);
        }
        Runnable worker = new DoorKeeperWorker(sip,flow);
        executor.execute(worker);
        return true;
    }

    public Flow executed(String sip) {
        synchronized(this) {
            Flow flow = executed.remove(sip);
            if (flow != null)
                executed.put(sip,flow);
            return flow;
        }
    }

    public void contextDestroyed(ServletContextEvent sce) {
        System.err.println("DoorKeeper: preparing to leave ...");
        executor.shutdown();
        while (!executor.isTerminated()) {
        }
        System.err.println("DoorKeeper: goodbye!");
    }
    
}
