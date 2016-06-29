package nl.mpi.tla.flat.deposit.action.handle.util;

import java.io.FileNotFoundException;
import java.io.IOException;

import nl.mpi.handle.util.HandleManager;
import nl.mpi.handle.util.implementation.HandleInfoProviderImpl;
import nl.mpi.handle.util.implementation.HandleManagerImpl;
import nl.mpi.handle.util.implementation.HandleParserImpl;
import nl.mpi.handle.util.implementation.HandleUtil;

public class HandleManagerFactory {

	//avoid instantiation of the class
	private HandleManagerFactory() {
		throw new AssertionError();
	}
	
	public static HandleManager getNewHandleManager(
			String handlePrefix, String handleAdminKeyFilePath, String handleAdminUserHandleIndex,
			String handleAdminUserHandle, String handleAdminPassword)
					throws FileNotFoundException, IOException {
		return new HandleManagerImpl(
				new HandleInfoProviderImpl(handlePrefix),
				new HandleParserImpl(handlePrefix),
				new HandleUtil(handleAdminKeyFilePath, handleAdminUserHandleIndex, handleAdminUserHandle, handleAdminPassword),
				handlePrefix);
	}
}
