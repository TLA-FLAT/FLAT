package nl.mpi.tla.flat;

import nl.mpi.tla.flat.deposit.Flow;

import java.io.File;
import java.util.HashMap;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.annotation.PostConstruct;
import javax.servlet.ServletContext;
import javax.ws.rs.DefaultValue;

import javax.ws.rs.GET;
import javax.ws.rs.PUT;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.Response.Status;
import javax.xml.transform.stream.StreamSource;
import net.sf.saxon.s9api.QName;
import net.sf.saxon.s9api.SaxonApiException;
import net.sf.saxon.s9api.XdmAtomicValue;
import net.sf.saxon.s9api.XdmDestination;
import net.sf.saxon.s9api.XdmValue;
import net.sf.saxon.s9api.XsltTransformer;
import nl.mpi.tla.flat.deposit.DepositException;
import nl.mpi.tla.flat.deposit.SIP;
import nl.mpi.tla.flat.deposit.action.CreateFOX;
import nl.mpi.tla.flat.deposit.util.Saxon;

@Path("/doorkeeper/{sip}")
public class DoorKeeperServlet {
    
    @Context
    private ServletContext servletContext;
    
    private Flow getFlow(Map<String,XdmValue> params) throws Exception {
        return new Flow(new File(servletContext.getInitParameter("doorkeeperConfig")),params);
    }

    @PUT
    @Produces("text/plain")
    public Response putSip(
            @PathParam("sip") String sip, 
            @DefaultValue("") @QueryParam("from") String start,
            @DefaultValue("") @QueryParam("to") String stop
    ) {
        DoorKeeperContextListener doorkeeperContext = (DoorKeeperContextListener)servletContext.getAttribute("DOORKEEPER");
        Flow flow = doorkeeperContext.executed(sip);
        if (flow==null) {
            Map<String,XdmValue> params = new HashMap();
            params.put("sip", new XdmAtomicValue(sip));
            String sipDir = "";
            try {
                flow = this.getFlow(params);
                if (start!=null && !start.isEmpty())
                    flow.setStart(start);
                if (stop!=null && !stop.isEmpty())
                    flow.setStop(stop);
                sipDir = flow.getContext().getProperty("work", "/tmp").toString();
                File sd = new File(sipDir);
                if (!sd.isDirectory()) {
                    return Response.status(Status.NOT_FOUND).entity("The SIP["+sip+"] directory["+sipDir+"] doesn't exist!").type("text/plain").build();
                }
            } catch (Exception ex) {
                return Response.status(Status.INTERNAL_SERVER_ERROR).entity(ex.getMessage()).type("text/plain").build();
            }
            if (doorkeeperContext.execute(sip,flow))
                return Response.status(Status.ACCEPTED).entity("sip["+sip+"] directory["+sipDir+"]").type("text/plain").build();
        }
        return Response.status(Status.CONFLICT).entity("ERROR: sip["+sip+"] is already being executed!"+(flow.getStatus()!=null?(flow.getStatus().booleanValue()?" And succeeded.":" And failed."):"")).type("text/plain").build();
    }

    @GET
    @Produces("text/plain")
    public Response getSip(@PathParam("sip") String sip) {
        DoorKeeperContextListener doorkeeperContext = (DoorKeeperContextListener)servletContext.getAttribute("DOORKEEPER");
        Flow flow = doorkeeperContext.executed(sip);
        if (flow == null)
            return Response.status(Status.NOT_FOUND).entity("ERROR: The SIP["+sip+"] isn't executed!").type("text/plain").build();
        File sd = new File(flow.getContext().getProperty("work", "/tmp").toString());
        File log = sd.toPath().resolve("./logs/user-log.xml").toFile();
        if (!log.isFile())
            log = null;
        Boolean status = flow.getStatus();
        XdmDestination destination = new XdmDestination();
        try {
            XsltTransformer wrap = Saxon.buildTransformer(this.getClass().getResource("/GETResult.xsl")).load();
            if (log!=null)
                wrap.setSource(new StreamSource(log));
            else
                wrap.setSource(new StreamSource(Flow.class.getResourceAsStream("/WorkspaceLog/empty-log.xml")));
            if (status!=null)
                wrap.setParameter(new QName("status"), new XdmAtomicValue(status.booleanValue()));
            if (flow.getContext().hasSIP()) {
                SIP ip = flow.getContext().getSIP();
                if (ip.hasPID())
                    wrap.setParameter(new QName("pid"), new XdmAtomicValue(ip.getPID()));
                if (ip.hasFID())
                    wrap.setParameter(new QName("fid"), new XdmAtomicValue(ip.getFID().toString().replaceAll("#.*", "")));
            }
            wrap.setDestination(destination);
            wrap.transform();
        } catch (SaxonApiException | DepositException ex) {
            Logger.getLogger(DoorKeeperServlet.class.getName()).log(Level.SEVERE, null, ex);
        }
        return Response.status((status==null?Status.ACCEPTED:Status.OK)).entity(destination.getXdmNode().asSource()).type("application/xml").build();
//        if (status == null)
//            return Response.status(Status.ACCEPTED).entity("The SIP["+sip+"] is being executed!").type("text/plain").build();
//        return Response.status(Status.OK).entity("The SIP["+sip+"] has been executed"+(status.booleanValue()?" succesfully!":", but failed!")).type("text/plain").build();
    }
    
}
