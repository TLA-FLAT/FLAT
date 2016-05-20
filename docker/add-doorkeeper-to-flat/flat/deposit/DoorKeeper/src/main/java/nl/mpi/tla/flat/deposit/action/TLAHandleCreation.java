package nl.mpi.tla.flat.deposit.action;

import java.io.File;
import java.io.IOException;
import java.net.URI;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import net.handle.hdllib.HandleException;
import nl.mpi.handle.util.HandleManager;
import nl.mpi.tla.flat.deposit.Context;
import nl.mpi.tla.flat.deposit.DepositException;
import nl.mpi.tla.flat.deposit.Resource;
import nl.mpi.tla.flat.deposit.action.handle.util.HandleManagerFactory;

public class TLAHandleCreation extends AbstractAction {

	private static final Logger logger = LoggerFactory.getLogger(TLAHandleCreation.class);
	
	@Override
	public boolean perform(Context context) throws DepositException {
		
		boolean allSuccessful = true;
		
		String fedoraServer = this.getParameter("fedoraServer");
		String handlePrefix = this.getParameter("handlePrefix");
		String handleAdminKeyFilePath = this.getParameter("handleAdminKeyFilePath");
		String handleAdminUserHandleIndex = this.getParameter("handleAdminUserHandleIndex");
		String handleAdminUserHandle = this.getParameter("handleAdminUserHandle");
		String handleAdminPassword = this.getParameter("handleAdminPassword");
		
		HandleManager handleManager = null;
		try {
			handleManager = HandleManagerFactory.getNewHandleManager(
					handlePrefix, handleAdminKeyFilePath, handleAdminUserHandleIndex, handleAdminUserHandle, handleAdminPassword);
		} catch (IOException ex) {
			StringBuilder message = new StringBuilder("Could not instantiate HandleManager");
			throwDepositException(message, ex);
		}
        
        String sipFid = context.getSIP().getFID().toString().replaceAll("#.*","");
        String sipDsid = context.getSIP().getFID().getRawFragment().replaceAll("@.*","");
        String sipAsof = context.getSIP().getFID().getRawFragment().replaceAll(".*@","");
        URI sipHandleTarget = URI.create(fedoraServer + "/objects/" + sipFid + "/datastreams/" + sipDsid + "/content?asOfDateTime=" + sipAsof);
        File sipBase = context.getSIP().getBase();
        URI sipPid = context.getSIP().getPID();
        
        logger.info("Creating handle[" + sipPid + "] -> URI[" + sipHandleTarget + "]");
        
        try {
			handleManager.assignHandle(sipBase, sipPid, sipHandleTarget);
		} catch (HandleException | IOException ex) {
			StringBuilder message = new StringBuilder("Error assigning handle '").append(context.getSIP().getPID()).append("', of SIP '").append(context.getSIP().getFID()).append("', to target '").append(sipHandleTarget).append("'.");
			throwDepositException(message, ex);
		}
        
        
        for (Resource res : context.getSIP().getResources()) {
            String resFid = res.getFID().toString().replaceAll("#.*","");
            String resDsid = res.getFID().getRawFragment().replaceAll("@.*","");
            String resAsof = res.getFID().getRawFragment().replaceAll(".*@","");
            URI resHandleTarget = URI.create(fedoraServer + "/objects/" + resFid + "/datastreams/" + resDsid + "/content?asOfDateTime=" + resAsof);
            File resFile = res.getFile();
            
            try {
            	URI resPid = res.getPID();
            	
            	logger.info("Creating handle[" + resPid + "] -> URI[" + resHandleTarget + "]");
            
				handleManager.assignHandle(resFile, resPid, resHandleTarget);
			} catch (HandleException | IOException ex) {
				StringBuilder message = new StringBuilder("Error assigning handle '").append(res.getPID()).append("', of resource '").append(res.getFID()).append("', to target '").append(resHandleTarget).append("'.");
				logger.error(message.toString(), ex);
				allSuccessful = false;
			} catch (DepositException ex) {
				logger.error(ex.getMessage(), ex);
				allSuccessful = false;
			}
        }
		
		return allSuccessful;
	}

	
	private void throwDepositException(StringBuilder message, Exception cause) throws DepositException {
		logger.error(message.toString(), cause);
		throw new DepositException(message.toString(), cause);
	}
}
