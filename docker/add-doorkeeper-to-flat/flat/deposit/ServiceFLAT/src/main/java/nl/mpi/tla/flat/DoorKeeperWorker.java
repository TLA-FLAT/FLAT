/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package nl.mpi.tla.flat;

import java.util.logging.Level;
import java.util.logging.Logger;
import nl.mpi.tla.flat.deposit.DepositException;
import nl.mpi.tla.flat.deposit.Flow;

/**
 *
 * @author menzowi
 */
public class DoorKeeperWorker implements Runnable {
    
    private final String sip;
    private final Flow flow;
    
    public DoorKeeperWorker(String sip,Flow flow){
        this.sip  = sip;
        this.flow = flow;
    }
    
    @Override
    public void run() {
        try {
            this.flow.run();
        } catch (DepositException ex) {
            Logger.getLogger(DoorKeeperWorker.class.getName()).log(Level.SEVERE, "DoorKeeper: depositing sip[{0}] failed!", sip);
            Logger.getLogger(DoorKeeperWorker.class.getName()).log(Level.SEVERE, "DoorKeeper: cause:", ex);
        }
    }
    
}
